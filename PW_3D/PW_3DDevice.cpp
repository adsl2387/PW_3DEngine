#include "PW_3DDevice.h"
#include <stdlib.h>
#include <stdio.h>

PW_3DDevice g_PW3DDevice;
PW_3DDevice* g_pPW3DDevice = &g_PW3DDevice;
PW_BOOL gBFlag = PW_FALSE;
PW_3DDevice::PW_3DDevice()
{
	m_pBitBuffer = NULL;
	m_pZBuffer = NULL;
	m_pShadowZBuffer = NULL;
	m_hWnd = NULL;
	m_hBitmap = NULL;
	m_ds = solid;
	m_bUseMaterial = false;
	m_bUseLight = false;
	m_bUseBiliner = false;
	m_bRayTrace = false;
	m_nDrawX = -1;
	m_nDrawY = -1;
	fnVertexShader = NULL;
	fnPixelShader = NULL;
	m_pShadowMapCamera = NULL;
}

PW_3DDevice::~PW_3DDevice()
{
	this->Release();
}



bool PW_3DDevice::Create(HWND hWnd, int iWidth, int iHeight, HWND hEdit)
{
	m_nMaxDepth = 5;
	m_bRayTrace = PW_FALSE;
	m_bWrite = 0;
	m_nCurNodePos = 0;
	m_hWnd = hWnd;
	m_hEdit = hEdit;
	m_iHeight = iHeight;
	m_iWidth = iWidth;
	m_fHeight = iHeight;
	m_fWidth = iWidth;
	
	HDC hDc = GetDC(m_hWnd);
	BITMAPINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.bmiHeader.biBitCount = 32;
	bi.bmiHeader.biHeight = iHeight;
	bi.bmiHeader.biWidth = iWidth;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biCompression = BI_RGB;
	m_hBitmap = CreateDIBSection(hDc, &bi, DIB_RGB_COLORS, (void**)&m_pBitBuffer, NULL, 0);
	if (!m_hBitmap)
	{
		return false;
	}
	m_pZBuffer = new PW_FLOAT[iWidth * iHeight];
	m_pShadowZBuffer = new PW_FLOAT[iWidth * iHeight];
	PW_FLOAT ftmp = 1.0f;
	DWORD* dtmp = reinterpret_cast<DWORD*>(&ftmp);
	DWORD tmp = *dtmp;
	QuadMemSet(m_pZBuffer, iHeight * iWidth * sizeof(DWORD), tmp);
	m_iFps = 0;
	m_4dBuffersize = 256;
	m_v4dBuffer = new PW_Vector4D[m_4dBuffersize];
	m_vNormalsBuffer = new PW_Vector3D[m_4dBuffersize];
	m_nIndexBuffer = new PW_INT[m_4dBuffersize];
	m_fVBuffer = new PW_FLOAT[m_4dBuffersize];
	m_fUBuffer = new PW_FLOAT[m_4dBuffersize];
	m_bShow = 0;
	m_curIndexPos = 0;
	m_curV4DPos = 0;
	m_pShadowMapCamera = new PW_OrthoCamera;
	//m_pShadowMapCamera = new PW_PerspectiveCamera;
	return true;
}

PW_RayTraceNode g_Node[10000];

void PW_3DDevice::Clear(PW_COLOR pwcolor, PW_FLOAT pwzbuffer)
{
	DWORD* dtmp = reinterpret_cast<DWORD*>(&pwzbuffer);
	DWORD tmp = *dtmp;
	if (!m_bRayTrace)
	{
		QuadMemSet(m_pZBuffer, m_iWidth * m_iHeight * sizeof(DWORD), tmp);

		QuadMemSet(m_pShadowZBuffer, m_iWidth * m_iHeight * sizeof(DWORD), tmp);
	}
	
	this->m_pMeshs.clear();
	
	QuadMemSet(m_pBitBuffer, m_iWidth * m_iHeight * sizeof(DWORD), pwcolor);
	m_fMaxZ = 0;
	m_fMinZ = 1;
	m_curV4DPos = 0;
	m_curIndexPos = 0;
	m_nCurNodePos = 0;
	//m_vLights.clear();
	//memset(g_Node, 0, sizeof(g_Node));
}

void PW_3DDevice::OutputHelpInfo()
{
	HDC hDc = GetDC(m_hWnd);
	HDC hMdc = CreateCompatibleDC(hDc);
	
	SelectObject(hMdc, m_hBitmap);
	BitBlt(hDc, 0, 0, m_iWidth, m_iHeight, hMdc, 0, 0, SRCCOPY);
	
	m_iFcount++;
	DWORD dwCur = GetTickCount();
	if (dwCur - m_lastTick >= 1000)
	{
		m_iFps = m_iFcount ;
		m_iFcount = 0;
		m_lastTick = dwCur;
	}
	SetBkMode(hDc, TRANSPARENT);
	char buffer[200];
	sprintf(buffer, "fps : %d, rotate angle :%.4f , use light: %d, use texture: %d, use material: %d, use bilinerfilter: %d"
		, m_iFps, m_fRotate / 2.f / PI, m_bUseLight, m_bUseTexture ,m_bUseMaterial, m_bUseBiliner);
	TextOut(hDc, 5, 0, buffer, strlen(buffer));

	TextOut(hDc, 5, 20, "z : rorate", strlen("z : rorate"));

	TextOut(hDc, 5, 40, "z : material", strlen("x : material"));

	TextOut(hDc, 5, 60, "g : light", strlen("g : light"));

	TextOut(hDc, 5, 80, "h : texture", strlen("h : texture"));

	TextOut(hDc, 5, 100, "j : texture filter", strlen("j : texture filter"));

	TextOut(hDc, 5, 120, "r : raytrace", strlen("r : raytrace"));

	TextOut(hDc, 5, 140, "t : wireframe", strlen("t : wireframe"));

	TextOut(hDc, 5, 160, "b : direction light", strlen("b : direction light"));

	TextOut(hDc, 5, 180, "n : point light", strlen("n : point light"));
	//DeleteObject(hMdc);
	DeleteDC(hMdc);
	ReleaseDC(m_hWnd, hDc);
}

void PW_3DDevice::Release()
{
	if (m_pZBuffer)
	{
		delete[] m_pZBuffer;
		m_pZBuffer = NULL;
	}
	if (m_pShadowZBuffer)
	{
		delete[] m_pShadowZBuffer;
		m_pShadowZBuffer = NULL;
	}
	if (m_hBitmap)
	{
		DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
	}
	if (m_v4dBuffer)
	{
		delete[] m_v4dBuffer;
		m_v4dBuffer = NULL;
		m_4dBuffersize = 0;
		delete[] m_vNormalsBuffer;
		m_vNormalsBuffer = NULL;
		delete[] m_nIndexBuffer;
		m_nIndexBuffer = NULL;
		m_curIndexPos = 0;
		m_curV4DPos = 0;
		delete[] m_fUBuffer;
		m_fUBuffer = NULL;
		delete[] m_fVBuffer;
		m_fVBuffer = NULL;
	}
	if (m_pShadowMapCamera)
	{
		delete m_pShadowMapCamera;
		m_pShadowMapCamera = NULL;
	}
}

void PW_3DDevice::Render()
{
	if (m_bRayTrace)
	{
		RayTrace();
	}
	else
		RenderScene();

	Update();
}

void PW_3DDevice::Update()
{
	OutputHelpInfo();
	Clear(PW_RGBA(127, 127, 127), 1.f);
}

void PW_3DDevice::DrawLineTexture(PW_POINT3D point1, PW_POINT3D point2, int isolid /* = 1 */)
{
	if (!m_pBitBuffer)
	{
		return;
	}
	PW_BOOL bPerCorrect = !m_pCamera->IsOrthoCamera();
	PW_BOOL bEnableZTest = m_dwRenderState & PW_RS_ENABLEZTEST;
	PW_BOOL bEnableZWrite = m_dwRenderState & PW_RS_ENABLEZWRITE;
	PW_BOOL bEnableColorWrite = m_dwRenderState & PW_RS_ENABLECOLORWRITE;
	m_bShow++;

	int dx = ROUND(point2.x) - ROUND(point1.x);
	int dy = ROUND(point2.y) - ROUND(point1.y);
	PW_FLOAT fvz1 = bPerCorrect ? 1.f / GetViewZ(point1.z) :point1.z;
	PW_FLOAT fvz2 = bPerCorrect ? 1.f / GetViewZ(point2.z) :point2.z;
	PW_FLOAT dz = fvz2 - fvz1;
	PW_FLOAT fRatio1 = bPerCorrect ? fvz1 : 1;
	PW_FLOAT fRatio2 = bPerCorrect ? fvz2 : 1;
	int steps;
	if (abs(dx) > abs(dy))
	{
		steps = abs(dx);
	}
	else
		steps = abs(dy);
	if (!(ROUND(point1.y) < 0 || ROUND(point1.y) >= m_iHeight || ROUND(point1.x) < 0 || ROUND(point1.x) >= m_iWidth))
	{
		if (GetValueOfZBuffer(ROUND(point1.x), ROUND(point1.y)) - point1.z> 0 || !bEnableZTest)
		{
			PW_COLOR pwColor = fnPixelShader(point1);
			/*if (m_bUseBiliner)
			{
				pwColor = point1.fP * m_texture->BiLinerGetColor(point1.u, point1.v);
			}
			else
				pwColor = point1.fP * m_texture->GetColor(point1.u, point1.v);*/
			if (bEnableColorWrite)
				SetValueOfCBuffer(ROUND(point1.x), ROUND(point1.y), pwColor);
			if (bEnableZWrite)
				SetValueOfZBuffer(ROUND(point1.x), ROUND(point1.y), point1.z);
		}	
	}
	if (steps == 0)
	{
		return;
	}
	PW_FLOAT fIncrementx = dx / (PW_FLOAT)steps;
	PW_FLOAT fIncrementy = dy / (PW_FLOAT)steps;
	PW_FLOAT fIncrementz = dz / steps;
	PW_COLORF fIncrementlp1 = (point2.fP * fRatio2 - point1.fP * fRatio1) / PW_FLOAT(steps);

	PW_FLOAT fIncrementu1 = (point2.u * fRatio2 - point1.u * fRatio1) / PW_FLOAT(steps);
	PW_FLOAT fIncrementv1 = (point2.v * fRatio2 - point1.v * fRatio1) / PW_FLOAT(steps);


	PW_FLOAT fx = point1.x;
	PW_FLOAT fy = point1.y;
	PW_FLOAT fz =  fvz1;

	PW_COLORF fLp1 = point1.fP * fRatio1;

	PW_FLOAT fU1 = point1.u * fRatio1;
	PW_FLOAT fV1 = point1.v * fRatio1;

	for (int i = 0; i < steps;++i)
	{
		if (isolid == 0)
		{
			i++;
			i++;
			fx += fIncrementx;
			fy += fIncrementy;
			fz += fIncrementz;

			fx += fIncrementx;
			fy += fIncrementy;
			fz += fIncrementz;

			fLp1 = fLp1 + fIncrementlp1;

			fU1 += fIncrementu1;
			fV1 += fIncrementv1;

		}

		fLp1 = fLp1 + fIncrementlp1;

		fU1 += fIncrementu1;
		fV1 += fIncrementv1;

		fx += fIncrementx ;
		fy += fIncrementy ;
		fz += fIncrementz ;

		if (ROUND(fy) < 0 || ROUND(fy) >= m_iHeight || ROUND(fx) <0 || ROUND(fx) >= m_iWidth)
		{
			continue;
		}
		PW_FLOAT fsz = bPerCorrect ? GetViewPortZ(1.f / fz) : fz;
		if (GetValueOfZBuffer(ROUND(fx), ROUND(fy)) - fsz > 0/*m_pZBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] - fz> 0*/ || !bEnableZTest)
		{
			PW_COLOR pwColor;
			PW_FLOAT fsu,fsv;
			PW_FLOAT fdz = bPerCorrect ? 1.f / fz : 1;
			fsu = fU1 * fdz;
			fsv = fV1 * fdz;
			PW_POINT3D in;
			in.fP = fLp1 * fdz;
			in.u = fsu;
			in.v = fsv;
			in.z = fsz;
			pwColor = fnPixelShader(in);
			//if (m_bUseBiliner)
			//{
			//	pwColor = fLp1 * fdz * m_texture->BiLinerGetColor(fsu, fsv);
			//}
			//else
			//	pwColor = fLp1 * fdz * m_texture->GetColor(fsu, fsv);
			if (bEnableColorWrite)
				SetValueOfCBuffer(ROUND(fx), ROUND(fy), pwColor);
			if (bEnableZWrite)
				SetValueOfZBuffer(ROUND(fx), ROUND(fy), fsz);
		}

	}
}


void PW_3DDevice::DrawLine3D(PW_POINT3D point1, PW_POINT3D point2)
{

}

void PW_3DDevice::DrawLine2D(PW_POINT3D point1, PW_POINT3D point2, int isolid, PW_BOOL bEnableZ)
{
	PW_BOOL bEnableZTest = m_dwRenderState & PW_RS_ENABLEZTEST;
	PW_BOOL bEnableZWrite = m_dwRenderState & PW_RS_ENABLEZWRITE;
	PW_BOOL bEnableColorWrite = m_dwRenderState & PW_RS_ENABLECOLORWRITE;
	if (!m_pBitBuffer)
	{
		return;
	}
	PW_BOOL bPersCorrect = !m_pCamera->IsOrthoCamera();
	//point1.y = m_iHeight - point1.y;
	//point2.y = m_iHeight - point2.y;
	int dx = ROUND(point2.x) - ROUND(point1.x);
	int dy = ROUND(point2.y) - ROUND(point1.y);
	PW_FLOAT fvz1 = bPersCorrect ? 1.f / GetViewZ(point1.z) : point1.z;
	PW_FLOAT fvz2 = bPersCorrect ? 1.f / GetViewZ(point2.z) : point2.z;
	PW_FLOAT fRatio1 = bPersCorrect ? fvz1 : 1;
	PW_FLOAT fRatio2 = bPersCorrect ? fvz2 : 1;
	PW_FLOAT dz = fvz2 - fvz1;
	int steps;
	if (abs(dx) > abs(dy))
	{
		steps = abs(dx);
	}
	else
		steps = abs(dy);
	if (!(ROUND(point1.y) < 0 || ROUND(point1.y) >= m_iHeight || ROUND(point1.x) < 0 || ROUND(point1.x) >= m_iWidth))
	{
		if (!bEnableZTest || GetValueOfZBuffer( ROUND(point1.x) ,ROUND(point1.y)) - point1.z> 0)
		{
			
			PW_COLOR pwColor = fnPixelShader(point1);

			if (bEnableColorWrite)
				SetValueOfCBuffer(ROUND(point1.x), ROUND(point1.y), pwColor); //m_pBitBuffer[ROUND(point1.y) * m_iWidth + ROUND(point1.x)] = pwColor;
			if (bEnableZWrite)
				SetValueOfZBuffer(ROUND(point1.x), ROUND(point1.y), point1.z);//m_pZBuffer[ROUND(point1.y) * m_iWidth + ROUND(point1.x)] = point1.z;

		}	
	}
	if (steps == 0)
	{
		return;
	}
	PW_FLOAT fIncrementx = dx / (PW_FLOAT)steps;
	PW_FLOAT fIncrementy = dy / (PW_FLOAT)steps;
	PW_FLOAT fIncrementz = dz / steps;
	PW_FLOAT fIncrementr = ((PW_FLOAT)(PW_RGBA_R(point2.pwColor)) * fRatio2 - PW_FLOAT(PW_RGBA_R(point1.pwColor)) * fRatio1) / PW_FLOAT(steps);
	PW_FLOAT fIncrementg = ((PW_FLOAT)(PW_RGBA_G(point2.pwColor)) * fRatio2 - PW_FLOAT(PW_RGBA_G(point1.pwColor)) * fRatio1) / PW_FLOAT(steps);
	PW_FLOAT fIncrementb = ((PW_FLOAT)(PW_RGBA_B(point2.pwColor)) * fRatio2 - PW_FLOAT(PW_RGBA_B(point1.pwColor)) * fRatio1) / PW_FLOAT(steps);

	PW_FLOAT fx = point1.x;
	PW_FLOAT fy = point1.y;
	PW_FLOAT fz = fvz1;
	PW_FLOAT fr = (PW_FLOAT)(PW_RGBA_R(point1.pwColor)) * fRatio1;
	PW_FLOAT fg = (PW_FLOAT)(PW_RGBA_G(point1.pwColor)) * fRatio1;
	PW_FLOAT fb = PW_FLOAT(PW_RGBA_B(point1.pwColor)) * fRatio1;

	for (int i = 0; i < steps;++i)
	{
		if (isolid == 0)
		{
			i++;
			i++;
			fx += fIncrementx;
			fy += fIncrementy;
			fz += fIncrementz;
			fr += fIncrementr;
			fg += fIncrementg;
			fb += fIncrementb;
			fx += fIncrementx;
			fy += fIncrementy;
			fz += fIncrementz;
			fr += fIncrementr;
			fg += fIncrementg;
			fb += fIncrementb;	
		}
		fx += fIncrementx ;
		fy += fIncrementy ;
		fz += fIncrementz ;
		fr += fIncrementr;
		fg += fIncrementg;
		fb += fIncrementb;
		if (ROUND(fy) < 0 || ROUND(fy) >= m_iHeight || ROUND(fx) <0 || ROUND(fx) >= m_iWidth)
		{
			continue;
		}
		PW_FLOAT fdz = bPersCorrect ? 1.f / fz : 1;
		PW_FLOAT fsz = bPersCorrect ? GetViewPortZ(fdz) : fz;
		if (!bEnableZTest || GetValueOfZBuffer(ROUND(fx), ROUND(fy)) - fsz> 0)
		{	
			PW_POINT3D pp;
			pp.x = fx;
			pp.y = fy;
			pp.z = fsz;

			pp.pwColor = PW_RGBA(ROUND(fr * fdz), ROUND(fg * fdz), ROUND(fb * fdz));
			PW_COLOR pwColor;//=  PW_RGBA(ROUND(fr * fdz), ROUND(fg * fdz), ROUND(fb * fdz));
			pwColor = fnPixelShader(pp);
			if (bEnableColorWrite)
				SetValueOfCBuffer(ROUND(fx), ROUND(fy), pwColor);
			if (bEnableZWrite)
				SetValueOfZBuffer(ROUND(fx), ROUND(fy), fsz);
		}
	
	}
}

void PW_3DDevice::DrawMesh(PW_Mesh& mesh)
{
	m_bShow = 0;
	PW_Matrix4D tran;
	PW_Matrix4D tran1;


	if (m_bRayTrace)
	{
		PW_MatrixProduct4D(m_pCamera->GetViewMat(), mesh.m_matAbsTM, tran);
		mesh.m_absoluteTM = tran;
		m_pMeshs.push_back(&mesh);
		//return;
	}
	else
	{
		mesh.m_absoluteTM = mesh.m_matAbsTM;
		m_pMeshs.push_back(&mesh);
	}
}

void PW_3DDevice::ComputeLight(PW_POINT3D& point, PW_Matrix4D& viewMat)
{
	PW_COLORF cAmbient,cDiffuse,cSpecular,cEmissive;
	PW_COLORF cP, cD,cS, cE;
	cP = 0;
	cD = 0;
	cS = 0;
	PW_FLOAT fSpe = 0;
	PW_COLORF cRet ;//光强
	if (m_bUseLight)
	{
		for (int i = 0; i < m_vLights.size();++i)
		{
			if (!m_vLights[i]->m_bOn)
			{
				continue;
			}
			PW_FLOAT par = 1.0f;
			PW_Vector3D lightdir = m_vLights[i]->m_vCurDir;
			if (m_vLights[i]->m_iLightType == pw_lt_spotlight)
			{
			}
			else if (m_vLights[i]->m_iLightType == pw_lt_pointlight)
			{
				//PW_Vector3D vP = m_vLights[i]->m_vPosition.MatrixProduct(viewMat);
				lightdir = lightdir - point;
			}
			else
			{

			}
			lightdir.Normalize();
			PW_FLOAT fRes = PW_DotProduct(lightdir, point.vNormal);
			if (fRes > 0)
			{
				cD = cD + m_vLights[i]->m_cDiffuse * fRes;
			}
			//相机总是在0,0,0
			PW_Vector3D vP = point * -1;
			vP.Normalize();
			vP = vP + lightdir;
			vP.Normalize();
			PW_FLOAT fTmp = PW_DotProduct(point.vNormal, vP);
			fTmp = pow(fTmp, m_material.fP);
			if (fTmp > 0)
			{
				cS = cS + m_vLights[i]->m_cSpecular * fTmp;
			}

			cP = cP + m_vLights[i]->m_cAmbient;
		}
		
	}
	else
	{
		cP = 0;
		cD = PW_COLORF(1,1,1,1);
		cS = 0;
	}
	
	//环境光
	cP = cP + m_Ambient;
	/*if (m_bUseTexture)
	{
		cAmbient = cP;
		cDiffuse = cD;
		cSpecular = cSpecular;
	}
	else */
	
	if (m_bUseMaterial)
	{
		cAmbient = m_material.cAmbient * cP;
		cDiffuse = m_material.cDiffuse * cD;
		cSpecular = m_material.cSpecularReflection * cS;	
	}
	else
	{
		cAmbient = cP;
		cDiffuse = cD;
		cSpecular = cS;
	}
	cRet = cAmbient + cDiffuse + cSpecular;
	point.fP = cRet;
	PW_COLOR pwRet = 0;
	if (m_bUseMaterial)
	{
		pwRet = cRet * (PW_COLOR)PW_RGBA(255, 255, 255);
	}
	else
	{
		pwRet = cRet * (PW_COLOR)point.pwColor;
	}
	//PW_CLAMP(pwRet, 0, 255);
	point.pwColor = pwRet;
}

PW_Vector4D VertexShader(PW_POINT3D& in)
{
	PW_Vector4D p1;
	PW_Matrix4D tan, projMatrix;
	projMatrix = g_pPW3DDevice->GetCamera()->GetProjMat();// this->m_pCamera->GetProjMat();
	//PW_MatrixProduct4D(projMatrix, g_pPW3DDevice->GetCamera()->GetViewMat(), tan);
	p1 = in.MatrixProduct(g_pPW3DDevice->GetCamera()->GetViewMat());
	//p1.NoneHomogeneous();
	p1.MatrixProduct(projMatrix);
	p1.NoneHomogeneous();

	in.vNormal = in.vNormal.MatrixProduct(g_pPW3DDevice->GetCamera()->GetViewMat(), PW_FALSE);
	in.vNormal.Normalize();
	g_pPW3DDevice->ComputeLight(in, g_pPW3DDevice->GetCamera()->GetViewMat());
	return p1;
}

PW_Vector4D ShadowMapVS(PW_POINT3D& in)
{
	PW_Vector4D p1;
	PW_Matrix4D tan, projMatrix;
	projMatrix = g_pPW3DDevice->GetShadowMapCamera()->GetProjMat();// this->m_pCamera->GetProjMat();
	//PW_MatrixProduct4D(projMatrix, g_pPW3DDevice->GetCamera()->GetViewMat(), tan);
	p1 = in.MatrixProduct(g_pPW3DDevice->GetShadowMapCamera()->GetViewMat());
	p1.MatrixProduct(projMatrix);
	//p1 = in.MatrixProduct(tan);
	p1.NoneHomogeneous();


	//in.vNormal = in.vNormal.MatrixProduct(g_pPW3DDevice->GetCamera()->GetViewMat(), PW_FALSE);
	//in.vNormal.Normalize();
	//g_pPW3DDevice->ComputeLight(in, g_pPW3DDevice->GetCamera()->GetViewMat());
	return p1;
}

PW_COLOR PixelShader(PW_POINT3D& in)
{
	PW_COLOR pwColor;
	PW_Texture* pTexture = g_pPW3DDevice->GetTexture();
	if (!pTexture)
	{
		return in.pwColor;
	}
	if (g_pPW3DDevice->UseBilinerTex())
	{
		pwColor = in.fP * pTexture->BiLinerGetColor(in.u, in.v);
	}
	else
		pwColor = in.fP * pTexture->GetColor(in.u, in.v);
	return pwColor;
}

PW_COLOR ShadowMapPS(PW_POINT3D& in)
{
	return 0xffffffff;
}

PW_Vector4D ReceiveShodowVS(PW_POINT3D& in)
{
	PW_Vector4D p1;
	PW_Matrix4D tan, projMatrix;
	projMatrix = g_pPW3DDevice->GetCamera()->GetProjMat();// this->m_pCamera->GetProjMat();
	PW_MatrixProduct4D(projMatrix, g_pPW3DDevice->GetCamera()->GetViewMat(), tan);

	p1 = in.MatrixProduct(tan);
	p1.NoneHomogeneous();


	in.vNormal = in.vNormal.MatrixProduct(g_pPW3DDevice->GetCamera()->GetViewMat(), PW_FALSE);
	in.vNormal.Normalize();
	g_pPW3DDevice->ComputeLight(in, g_pPW3DDevice->GetCamera()->GetViewMat());
	return p1;
}

PW_COLOR ReceiveShadowPS(PW_POINT3D& in)
{
	//PW_Vector3D vTest(10, 10, 20);
	//PW_Vector4D vT4 = vTest.MatrixProduct(g_pPW3DDevice->GetShadowMapCamera()->GetViewMat());
	//vT4.MatrixProduct(g_pPW3DDevice->GetShadowMapCamera()->GetProjMat());
	//vT4.NoneHomogeneous();
	//vT4.MatrixProduct(g_pPW3DDevice->GetViewportMatrix());
	//PW_Vector3D vTT = vT4;

	//PW_Vector3D vORI;
	//g_pPW3DDevice->GetViewPos(vTT, vORI);
	//vORI = vORI.MatrixProduct(g_pPW3DDevice->GetShadowMapCamera()->GetInverseViewMat());

	PW_COLOR pwColor;
	PW_Texture* pTexture = g_pPW3DDevice->GetTexture();
	PW_Vector3D vViewPos;
	PW_POINT3D tmpPoint = in;
	//tmpPoint.y = g_pPW3DDevice->GetViewPortHeight() - in.y;
	g_pPW3DDevice->GetViewPos(tmpPoint, vViewPos);
	PW_Vector4D vWorldPos = vViewPos.MatrixProduct(g_pPW3DDevice->GetCamera()->GetInverseViewMat());
	vWorldPos.MatrixProduct(g_pPW3DDevice->GetShadowMapCamera()->GetViewMat());
	vWorldPos.MatrixProduct(g_pPW3DDevice->GetShadowMapCamera()->GetProjMat());
	vWorldPos.MatrixProduct(g_pPW3DDevice->GetViewportMatrix());
	PW_FLOAT fShadowZ = g_pPW3DDevice->GetValueOfShadowZBuffer(ROUND(vWorldPos.x), ROUND(/*g_pPW3DDevice->GetViewPortHeight() -*/ vWorldPos.y));

	PW_COLORF fShadowValue = PW_COLORF(1.f, 1.f, 1.f, 1.f);

	if (fShadowZ < vWorldPos.z)
	{
		fShadowValue = fShadowValue * 0.5f;
	}

	//if (!pTexture)
	{
		return  fShadowValue * in.pwColor;
	}
	if (g_pPW3DDevice->UseBilinerTex())
	{
		pwColor = in.fP * fShadowValue * pTexture->BiLinerGetColor(in.u, in.v);
	}
	else
		pwColor = in.fP * fShadowValue * pTexture->GetColor(in.u, in.v);
	return pwColor;
}

void PW_3DDevice::ViewPortClamp(PW_Vector4D& p1)
{
	p1.x = fmax(0.f, p1.x);
	p1.y = fmax(0.f, p1.y);
	p1.x = fmin(m_iWidth, p1.x);
	p1.y = fmin(m_iHeight, p1.y);
}

//观察坐标系中的点
void PW_3DDevice::DrawTriPrimitive(PW_POINT3D point1, PW_POINT3D point2, PW_POINT3D point3, PW_COLOR color, int ds)
{
	PW_Vector3D pView1 = point1.MatrixProduct(m_pCamera->GetViewMat());
	PW_Vector3D pView2 = point2.MatrixProduct(m_pCamera->GetViewMat());
	PW_Vector3D pView3 = point3.MatrixProduct(m_pCamera->GetViewMat());


	PW_Vector3D v = pView2 - pView1;
	PW_Vector3D u = pView3 - pView2;
	PW_Vector3D dir(0, 0, 0);
	if (m_pCamera->IsOrthoCamera())
	{
		dir = PW_Vector3D(0.f, 0.f, -1.f);
	}
	else
	{
		dir = dir - pView1;
	}
	
	PW_Vector3D h;
	PW_CrossProduct(v, u, h);
	PW_FLOAT dotRes = PW_DotProduct(h, dir);

	//消除背面
	if (dotRes <= 0)
	{
		return;
	}

	PW_Vector4D p1, p2, p3;
	p1 = fnVertexShader(point1);
	p2 = fnVertexShader(point2);
	p3 = fnVertexShader(point3);
 	p1.MatrixProduct(m_viewportMatrix);
	p2.MatrixProduct(m_viewportMatrix);
	p3.MatrixProduct(m_viewportMatrix);

	//ViewPortClamp(p1);
	//ViewPortClamp(p2);
	//ViewPortClamp(p3);

	PW_BOOL bEnableZTest = m_dwRenderState & PW_RS_ENABLEZTEST;
	if (ds == wireframe)
	{
		DrawLine2D(PW_POINT3D(p1, point1.pwColor, point1.vNormal, point1.u, point1.v, point1.fP)
			, PW_POINT3D(p2, point2.pwColor, point2.vNormal, point2.u, point2.v, point2.fP), bEnableZTest);
		DrawLine2D(PW_POINT3D(p2, point2.pwColor, point2.vNormal, point2.u, point2.v, point2.fP)
			, PW_POINT3D(p3, point3.pwColor, point3.vNormal, point3.u, point3.v, point3.fP), bEnableZTest);
		DrawLine2D(PW_POINT3D(p3, point3.pwColor, point3.vNormal, point3.u, point3.v, point3.fP)
			, PW_POINT3D(p1, point1.pwColor, point1.vNormal, point1.u, point1.v, point1.fP), bEnableZTest);
	}
	else
	{
		DrawTriangle(PW_POINT3D(p1, point1.pwColor, PW_Vector3D(), point1.u, point1.v, point1.fP)
			, PW_POINT3D(p2, point2.pwColor, PW_Vector3D(), point2.u, point2.v, point2.fP)
			, PW_POINT3D(p3, point3.pwColor, PW_Vector3D(), point3.u, point3.v, point3.fP));		
	}
	
}

void PW_3DDevice::DrawTriangle(PW_Vector4D point1, PW_Vector4D point2, PW_Vector4D point3)
{
	PW_FLOAT fW1 = point1.w;
	PW_FLOAT fW2 = point2.w;
	PW_FLOAT fW3 = point3.w;

	point1.NoneHomogeneous();
	point1.MatrixProduct(m_viewportMatrix);

}

void PW_3DDevice::DrawTriangle(PW_POINT3D point1, PW_POINT3D point2, PW_POINT3D point3)
{
	PW_BOOL bEnableZTest = m_dwRenderState & PW_RS_ENABLEZTEST;
	PW_BOOL bPerspectiveCorrect = !m_pCamera->IsOrthoCamera();
	
	PW_POINT3D pps[3];
	pps[0] = point1;
	if (point2.y < pps[0].y)
	{
		pps[1] = pps[0];
		pps[0] = point2;
	}
	else
	{
		pps[1] = point2;
	}
	int i = 1;
	while (i >= 0 && point3.y < pps[i].y)
	{
		pps[i + 1] = pps[i];
		i--;
	}
	pps[i + 1] = point3;
	m_bWrite ++;

	int dy1 = ROUND(pps[1].y) - ROUND(pps[0].y);
	int dy2 = ROUND(pps[2].y) - ROUND(pps[0].y);
	PW_FLOAT fZ = GetViewZ(pps[0].z);
	PW_FLOAT fZ1 = GetViewZ(pps[1].z);
	PW_FLOAT fZ2 = GetViewZ(pps[2].z);

	PW_FLOAT fInterZ1 = bPerspectiveCorrect ? 1.f / fZ1 : pps[1].z;
	PW_FLOAT fInterZ2 = bPerspectiveCorrect ? 1.f / fZ2 : pps[2].z;
	PW_FLOAT fInterZ = bPerspectiveCorrect ? 1.f / fZ : pps[0].z;
	PW_FLOAT fRatioZ1 = bPerspectiveCorrect ? fInterZ1 : 1;
	PW_FLOAT fRatioZ2 = bPerspectiveCorrect ? fInterZ2 : 1;
	PW_FLOAT fRatioZ = bPerspectiveCorrect ? fInterZ : 1;
	int cury = ROUND(pps[0].y);
	PW_POINT3D leftPoint, rightPoint;
	PW_FLOAT fZl,fZr;
	if (dy1 > 0)
	{
		PW_FLOAT fIncrementx1 =(pps[1].x - pps[0].x) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementx2 = (pps[2].x - pps[0].x) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementz1 = (fInterZ1 - fInterZ) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementz2 = (fInterZ2 - fInterZ) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementr1 = ((PW_FLOAT)PW_RGBA_R(pps[1].pwColor) * fRatioZ1 - (PW_FLOAT)PW_RGBA_R(pps[0].pwColor) * fRatioZ) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementg1 = ((PW_FLOAT)PW_RGBA_G(pps[1].pwColor) * fRatioZ1 - (PW_FLOAT)PW_RGBA_G(pps[0].pwColor) * fRatioZ) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementb1 = ((PW_FLOAT)PW_RGBA_B(pps[1].pwColor) * fRatioZ1 - (PW_FLOAT)PW_RGBA_B(pps[0].pwColor) * fRatioZ) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementr2 = ((PW_FLOAT)PW_RGBA_R(pps[2].pwColor) * fRatioZ2 - (PW_FLOAT)PW_RGBA_R(pps[0].pwColor) * fRatioZ) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementg2 = ((PW_FLOAT)PW_RGBA_G(pps[2].pwColor) * fRatioZ2 - (PW_FLOAT)PW_RGBA_G(pps[0].pwColor) * fRatioZ) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementb2 = ((PW_FLOAT)PW_RGBA_B(pps[2].pwColor) * fRatioZ2 - (PW_FLOAT)PW_RGBA_B(pps[0].pwColor) * fRatioZ) / PW_FLOAT(dy2);
		PW_COLORF fIncrementlp1 = (pps[1].fP * fRatioZ1 - pps[0].fP * fRatioZ) / PW_FLOAT(dy1);
		PW_COLORF fIncrementlp2 = (pps[2].fP * fRatioZ2 - pps[0].fP * fRatioZ) / PW_FLOAT(dy2);
		PW_FLOAT fDu1 = (pps[1].u * fRatioZ1 - pps[0].u * fRatioZ) / PW_FLOAT(dy1);
		PW_FLOAT fDu2 = (pps[2].u * fRatioZ2 - pps[0].u * fRatioZ) / PW_FLOAT(dy2);
		PW_FLOAT fDv1 = (pps[1].v * fRatioZ1 - pps[0].v * fRatioZ) / PW_FLOAT(dy1);
		PW_FLOAT fDv2 = (pps[2].v * fRatioZ2 - pps[0].v * fRatioZ) / PW_FLOAT(dy2);
		if (fIncrementx1 > fIncrementx2)
		{
			PW_FLOAT fTmp;
			PW_SWAP(fIncrementx1, fIncrementx2, fTmp);
			PW_SWAP(fIncrementr1, fIncrementr2, fTmp);
			PW_SWAP(fIncrementg1, fIncrementg2, fTmp);
			PW_SWAP(fIncrementb1, fIncrementb2, fTmp);
			PW_SWAP(fIncrementz1, fIncrementz2, fTmp);
			PW_COLORF ffTmp;
			PW_SWAP(fIncrementlp1, fIncrementlp2, ffTmp);
			PW_SWAP(fDv1, fDv2, fTmp);
			PW_SWAP(fDu1, fDu2, fTmp);
			
		}
		PW_FLOAT fXl = (pps[0].x);
		PW_FLOAT fXr = (pps[0].x);
		fZl = fInterZ;
		fZr = fInterZ;
		
		PW_FLOAT fR1 = (PW_FLOAT)PW_RGBA_R(pps[0].pwColor) * fRatioZ;
		PW_FLOAT fG1 = (PW_FLOAT)PW_RGBA_G(pps[0].pwColor) * fRatioZ;
		PW_FLOAT fB1 = (PW_FLOAT)PW_RGBA_B(pps[0].pwColor) * fRatioZ;
		PW_FLOAT fR2 = (PW_FLOAT)PW_RGBA_R(pps[0].pwColor) * fRatioZ;
		PW_FLOAT fG2 = (PW_FLOAT)PW_RGBA_G(pps[0].pwColor) * fRatioZ;
		PW_FLOAT fB2 = (PW_FLOAT)PW_RGBA_B(pps[0].pwColor) * fRatioZ;
		PW_COLORF fLp1 = pps[0].fP * fRatioZ;
		PW_COLORF fLp2 = pps[0].fP * fRatioZ;
		PW_FLOAT fU1 = pps[0].u * fRatioZ;
		PW_FLOAT fV1 = pps[0].v * fRatioZ;
		PW_FLOAT fU2 = pps[0].u * fRatioZ;
		PW_FLOAT fV2 = pps[0].v * fRatioZ;
		int curY = ROUND(pps[0].y);

		if (m_bUseTexture)
		{
			DrawLineTexture(pps[0], pps[0]);
		}
		else
			DrawLine2D(pps[0], pps[0], bEnableZTest);
		
		for (int k = 0; k < dy1;++k)
		{
			fXl += fIncrementx1;
			fXr += fIncrementx2;
			fZl += fIncrementz1;
			fZr += fIncrementz2;
			curY++;
			fR1 += fIncrementr1;
			fG1 += fIncrementg1;
			fB1 += fIncrementb1;
			fR2 += fIncrementr2;
			fG2 += fIncrementg2;
			fB2 += fIncrementb2;


			fLp1 = fLp1 + fIncrementlp1;
			fLp2 = fLp2 + fIncrementlp2;
			fU1 += fDu1;
			fV1 += fDv1;
			fU2 += fDu2;
			fV2 += fDv2;
			if (k == dy1 - 1)
			{
				leftPoint.x = fXl;
				leftPoint.y = curY;
				leftPoint.z = bPerspectiveCorrect ? GetViewPortZ(1.f / fZl) : (fZl);
				leftPoint.fP = bPerspectiveCorrect ? fLp1 / fZl : fLp1;
				leftPoint.u = bPerspectiveCorrect ? fU1 / fZl : fU1;
				leftPoint.v = bPerspectiveCorrect ? fV1 / fZl : fV1;
				leftPoint.pwColor = PW_RGBA(ROUND(bPerspectiveCorrect? fR1 / fZl : fR1), ROUND(bPerspectiveCorrect ? fG1 / fZl : fG1), ROUND(bPerspectiveCorrect ? fB1 / fZl : fB1));
				rightPoint.x = fXr;
				rightPoint.y = curY;
				rightPoint.pwColor = PW_RGBA(ROUND(bPerspectiveCorrect ? fR2 / fZr : fR2), ROUND(bPerspectiveCorrect ? fG2 / fZr : fG2), ROUND(bPerspectiveCorrect ? fB2 / fZr : fB2));
				rightPoint.z = bPerspectiveCorrect ? GetViewPortZ(1.f / fZr) : GetViewPortZ(fZr);
				rightPoint.fP = bPerspectiveCorrect ? fLp2 / fZr : fLp2;
				rightPoint.u = bPerspectiveCorrect ? fU2 / fZr : fU2;
				rightPoint.v = bPerspectiveCorrect ? fV2 / fZr : fV2;
			}
			PW_POINT3D p1, p2;
			p1.x = fXl;
			p1.y = curY;
			p1.z = bPerspectiveCorrect ? GetViewPortZ(1.f / fZl) :  fZl;
			p1.fP = bPerspectiveCorrect ? fLp1 / fZl : fLp1;
			p1.u = bPerspectiveCorrect ? fU1 / fZl : fU1;
			p1.v = bPerspectiveCorrect ? fV1 / fZl : fV1;
			p1.pwColor = PW_RGBA(ROUND(bPerspectiveCorrect ? fR1 / fZl : fR1), ROUND(bPerspectiveCorrect ? fG1 / fZl : fG1), ROUND(bPerspectiveCorrect ? fB1 / fZl : fB1));
			p2.x = fXr;
			p2.y = curY;
			p2.z = bPerspectiveCorrect ? GetViewPortZ(1.f / fZr) : fZr;
			p2.pwColor = PW_RGBA(ROUND(bPerspectiveCorrect ? fR2 / fZr: fR2), ROUND(bPerspectiveCorrect ? fG2 / fZr : fG2), ROUND(bPerspectiveCorrect ? fB2 / fZr : fB2));
			p2.fP = bPerspectiveCorrect ? fLp2 / fZr : fLp2;
			p2.u = bPerspectiveCorrect ? fU2 / fZr : fU2;
			p2.v = bPerspectiveCorrect ? fV2 / fZr : fV2;
			if (m_bUseTexture)
			{
				DrawLineTexture(p1, p2);
			}
			else
				DrawLine2D(p1, p2);
		}
		
	}
	else
	{
		if (pps[0].x < pps[1].x)
		{
			leftPoint = pps[0];
			rightPoint = pps[1];
			fZl = bPerspectiveCorrect ? 1 / fZ : pps[0].z;
			fZr = bPerspectiveCorrect ? 1 / fZ1 : pps[1].z;
		}
		else
		{
			leftPoint = pps[1];
			rightPoint = pps[0];
			fZl = bPerspectiveCorrect ? 1 / fZ1 : pps[1].z;
			fZr = bPerspectiveCorrect ? 1 / fZ : pps[0].z;
		}	
	}
	if (dy2 != 0 && dy2 > dy1)
	{
		PW_FLOAT fRatiol = bPerspectiveCorrect ? fZl : 1;
		PW_FLOAT fRatior = bPerspectiveCorrect ? fZr : 1;
		PW_FLOAT fRatio2 = bPerspectiveCorrect ? 1.f / fZ2 : 1;
		dy1 = ROUND(pps[2].y) - ROUND(leftPoint.y);
		dy2 = ROUND(pps[2].y) - ROUND(rightPoint.y) ;
		PW_FLOAT fIncrementx1 = (pps[2].x - leftPoint.x) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementx2 = (pps[2].x - rightPoint.x) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementz1 = ((bPerspectiveCorrect ? 1.f / fZ2 : pps[2].z) - fZl) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementz2 = ((bPerspectiveCorrect ? 1.f / fZ2 : pps[2].z) - fZr) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementr1 = ((PW_FLOAT)PW_RGBA_R(pps[2].pwColor) * fRatio2 - (PW_FLOAT)PW_RGBA_R(leftPoint.pwColor) * fRatiol) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementg1 = ((PW_FLOAT)PW_RGBA_G(pps[2].pwColor) * fRatio2 - (PW_FLOAT)PW_RGBA_G(leftPoint.pwColor) * fRatiol) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementb1 = ((PW_FLOAT)PW_RGBA_B(pps[2].pwColor) * fRatio2 - (PW_FLOAT)PW_RGBA_B(leftPoint.pwColor) * fRatiol) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementr2 = ((PW_FLOAT)PW_RGBA_R(pps[2].pwColor) * fRatio2 - (PW_FLOAT)PW_RGBA_R(rightPoint.pwColor) * fRatior) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementg2 = ((PW_FLOAT)PW_RGBA_G(pps[2].pwColor) * fRatio2 - (PW_FLOAT)PW_RGBA_G(rightPoint.pwColor) * fRatior) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementb2 = ((PW_FLOAT)PW_RGBA_B(pps[2].pwColor) * fRatio2 - (PW_FLOAT)PW_RGBA_B(rightPoint.pwColor) * fRatior) / PW_FLOAT(dy2);
		PW_COLORF fIncrementlp1 = (pps[2].fP * fRatio2 - leftPoint.fP * fRatiol) / PW_FLOAT(dy1);
		PW_COLORF fIncrementlp2 = (pps[2].fP * fRatio2 - rightPoint.fP * fRatior) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementu1 = (pps[2].u * fRatio2 - leftPoint.u * fRatiol) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementv1 = (pps[2].v * fRatio2 - leftPoint.v * fRatiol) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementu2 = (pps[2].u * fRatio2 - rightPoint.u * fRatior) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementv2 = (pps[2].v * fRatio2 - rightPoint.v * fRatior) / PW_FLOAT(dy2);
		int steps = dy1;
		PW_FLOAT fCurR1 = (PW_FLOAT)PW_RGBA_R(leftPoint.pwColor) * fRatiol;
		PW_FLOAT fCurG1 = (PW_FLOAT)PW_RGBA_G(leftPoint.pwColor) * fRatiol;
		PW_FLOAT fCurB1 = (PW_FLOAT)PW_RGBA_B(leftPoint.pwColor) * fRatiol;
		PW_FLOAT fCurR2 = (PW_FLOAT)PW_RGBA_R(rightPoint.pwColor) * fRatior;
		PW_FLOAT fCurG2 = (PW_FLOAT)PW_RGBA_G(rightPoint.pwColor) * fRatior;
		PW_FLOAT fCurB2 = (PW_FLOAT)PW_RGBA_B(rightPoint.pwColor) * fRatior;
		PW_FLOAT fx1 = (leftPoint.x);
		PW_FLOAT fx2 = (rightPoint.x);

		PW_COLORF flp1 = leftPoint.fP * fRatiol;
		PW_COLORF flp2 = rightPoint.fP * fRatior;
		PW_FLOAT fU1 = leftPoint.u * fRatiol;
		PW_FLOAT fV1 = leftPoint.v * fRatiol;
		PW_FLOAT fU2 = rightPoint.u * fRatior;
		PW_FLOAT fV2 = rightPoint.v * fRatior;
		int curY = ROUND(leftPoint.y);
		for (int k = 0; k < steps;++k)
		{
			fx1 += fIncrementx1;
			fx2 += fIncrementx2;
			fZl += fIncrementz1;
			fZr += fIncrementz2;
			flp1 = flp1 + fIncrementlp1;
			flp2 = flp2 + fIncrementlp2;
			fU1 += fIncrementu1;
			fV1 += fIncrementv1;
			fU2 += fIncrementu2;
			fV2 += fIncrementv2;
			fCurR1 += fIncrementr1;
			fCurB1 += fIncrementb1;
			fCurG1 += fIncrementg1;
			fCurR2 += fIncrementr2;
			fCurB2 += fIncrementb2;
			fCurG2 += fIncrementg2;
			
			curY++;
			PW_POINT3D p1, p2;
			p1.x = fx1;
			p1.y = curY;
			p1.z = bPerspectiveCorrect ? GetViewPortZ(1.f / fZl) : fZl;
			p1.fP = bPerspectiveCorrect ? flp1 / fZl : flp1;
			p1.u = bPerspectiveCorrect ? fU1 / fZl : fU1;
			p1.v = bPerspectiveCorrect ? fV1 / fZl : fV1;

			p1.pwColor = PW_RGBA(ROUND(bPerspectiveCorrect ? fCurR1 / fZl : fCurR1), ROUND(bPerspectiveCorrect ? fCurG1 / fZl : fCurG1), ROUND(bPerspectiveCorrect ? fCurB1 / fZl : fCurB1));
			p2.x = fx2;
			p2.y = curY;
			p2.z = bPerspectiveCorrect ? GetViewPortZ(1.f / fZr) : fZr;
			p2.fP = bPerspectiveCorrect ? flp2 / fZr : flp2;
			p2.u = bPerspectiveCorrect ? fU2 / fZr : fU2;
			p2.v = bPerspectiveCorrect ? fV2 / fZr : fV2;
			p2.pwColor = PW_RGBA(ROUND(bPerspectiveCorrect ? fCurR2 / fZr : fCurR2), ROUND(bPerspectiveCorrect ? fCurG2 / fZr : fCurG2), ROUND(bPerspectiveCorrect ? fCurB2 / fZr : fCurB2));
			if (m_bUseTexture)
			{
				DrawLineTexture(p1, p2);
			}
			else
				DrawLine2D(p1, p2);
		}
	}
}

PW_Vector4D PW_3DDevice::GetOriPos(PW_FLOAT x, PW_FLOAT y, PW_FLOAT z)
{
	PW_Vector3D v3d;
	PW_FLOAT x1 = (x + x) / m_fWidth - 1.0f;
	PW_FLOAT y1 = 1.0f - (y + y) / m_fHeight;
	PW_FLOAT z1 = (z + z) - 1.0f;
	PW_Matrix4D projMatrix = this->m_pCamera->GetProjMat();
	v3d.z = projMatrix[2][3] / (z1 - projMatrix[2][2]);
	v3d.x = x1 * v3d.z;
	v3d.y = y1 * v3d.z;
	v3d.Normalize();	
	return v3d;
}

void PW_3DDevice::DrawEllipse(PW_FLOAT x , PW_FLOAT y, PW_FLOAT a, PW_FLOAT b, PW_COLOR pwColor /* = PW_RGB(255 , 255, 255)*/)
{
	PW_FLOAT fCurx = 0.f;
	PW_FLOAT fCury = -b;
	SetPixel(x + fCurx, y + fCury, pwColor);
	PW_FLOAT fdydx = b * fCurx / a / fCury;
	while (fabs(fdydx) < 1.f)
	{
		fCurx += 1.f;
		if (fCurx * fCurx / (a * a) + (fCury + 0.5f) * (fCury + 0.5f) / (b * b) - 1.f > 0)
		{
			fCury += 1.f;
		}
		SetPixel(x + fCurx, y + fCury, pwColor);
		fdydx = b * b* fCurx / (a * a) / fCury;
	}
	while (fCury <= 0)
	{
		fCury += 1.f;
		if ((fCurx + 0.5f) * (fCurx + 0.5f) / (a * a) + (fCury ) * (fCury ) / (b * b) - 1.f < 0)
		{
			fCurx += 1.f;
		}
		SetPixel(x + fCurx, y + fCury, pwColor);
	}
	fdydx = b * b* fCurx / (a * a) / fCury;
	while (fabs(fdydx) > 1.f)
	{
		fCury += 1.f;
		if ((fCurx - 0.5f) * (fCurx - 0.5f) / (a * a) + (fCury)* (fCury) / (b * b) - 1.f > 0)
		{
			fCurx -= 1.f;
		}
		SetPixel(x + fCurx, y + fCury, pwColor);
		fdydx = b * b* fCurx / (a * a) / fCury;
	}
	while (fCurx >= 0.f)
	{
		fCurx -= 1.f;
		if (fCurx * fCurx / (a * a) + (fCury + 0.5f) * (fCury + 0.5f) / (b * b) - 1.f < 0)
		{
			fCury += 1.f;
		}
		SetPixel(x + fCurx, y + fCury, pwColor);
	}
	fdydx = b * b* fCurx / (a * a) / fCury;
	while (fabs(fdydx) < 1.f)
	{
		fCurx -= 1.f;
		if (fCurx * fCurx / (a * a) + (fCury - 0.5f) * (fCury - 0.5f) / (b * b) - 1.f > 0)
		{
			fCury -= 1.f;
		}
		SetPixel(x + fCurx, y + fCury, pwColor);
		fdydx = b * b* fCurx / (a * a) / fCury;
	}
	while (fCury >= 0.f)
	{
		fCury -= 1.f;
		if ((fCurx - 0.5f) * (fCurx - 0.5f) / (a * a) + (fCury)* (fCury) / (b * b) - 1.f < 0)
		{
			fCurx -= 1.f;
		}
		SetPixel(x + fCurx, y + fCury, pwColor);
	}
	fdydx = b * b* fCurx / (a * a) / fCury;
	while (fabs(fdydx) > 1.f)
	{

		fCury -= 1.f;
		if ((fCurx + 0.5f) * (fCurx + 0.5f) / (a * a) + (fCury) * (fCury) / (b * b) - 1.f > 0)
		{
			fCurx += 1.f;
		}
		SetPixel(x + fCurx, y + fCury, pwColor);
		fdydx = b * b* fCurx / (a * a) / fCury;
	}
	while (fCurx <= 0.f)
	{
		fCurx += 1.f;
		if ((fCurx) * (fCurx) / (a * a) + (fCury - 0.5f)* (fCury - 0.5f) / (b * b) - 1.f < 0)
		{
			fCury -= 1.f;
		}
		SetPixel(x + fCurx, y + fCury, pwColor);
	}
}

void PW_3DDevice::DrawCircle(PW_FLOAT x, PW_FLOAT y, PW_FLOAT r, PW_COLOR pwColor /* = PW_RGB(255 ,255,255) */)
{
	r = ROUND(r);
	x = ROUND(x);
	y = ROUND(y);
	PW_FLOAT r2 = r * r;
	PW_FLOAT xk = x - r;
	PW_FLOAT yk = y;
	SetPixel(xk, yk, pwColor);
	yk -= 1.f;
	while (y - yk <= x - xk)
	{
		if ((xk - x  + 0.5f )* (xk - x + 0.5f) + (yk - y) * (yk - y) - r2 <= 0)
		{
			SetPixel(xk, yk, pwColor);
		}
		else
		{
			xk += 1.f;
			SetPixel(xk, yk, pwColor);
		}
		yk -= 1.f;
	}
	while (xk <= x)
	{
		if ((xk - x )* (xk - x) + (yk - 0.5f - y) * (yk - 0.5f - y) - r2 >= 0)
		{
			SetPixel(xk, yk, pwColor);
		}
		else
		{
			yk -= 1.f;
			SetPixel(xk, yk, pwColor);
		}
		xk += 1.f;
	}
	
	while (xk - x <= y - yk)
	{
		if ((xk - x)* (xk - x) + (yk + 0.5f - y) * (yk + 0.5f - y) - r2 <= 0)
		{
			SetPixel(xk, yk, pwColor);
		}
		else
		{
			yk += 1.f;
			SetPixel(xk, yk, pwColor);
		}
		xk += 1.f;
	}
	while (yk <= y)
	{
		if ((xk - x + 0.5f)* (xk - x + 0.5f) + (yk - y) * (yk - y) - r2 >= 0)
		{
			SetPixel(xk, yk, pwColor);
		}
		else
		{
			xk += 1.f;
			SetPixel(xk, yk, pwColor);
		}
		yk += 1.f;
	}
	//xia ban yuan
	yk = y + 1.f;
	xk = x - r;
	while (yk - y <= x - xk)
	{
		if ((xk - x + 0.5f)* (xk - x + 0.5f) + (yk - y) * (yk - y) - r2 <= 0)
		{
			SetPixel(xk, yk, pwColor);
		}
		else
		{
			xk += 1.f;
			SetPixel(xk, yk, pwColor);
		}
		yk += 1.f;
	}
	while (xk <= x)
	{
		if ((xk - x)* (xk - x) + (yk + 0.5f - y) * (yk + 0.5f - y) - r2 >= 0)
		{
			SetPixel(xk, yk, pwColor);
		}
		else
		{
			yk += 1.f;
			SetPixel(xk, yk, pwColor);
		}
		xk += 1.f;
	}

	while (xk - x <= yk - y)
	{
		if ((xk - x)* (xk - x) + (yk - 0.5f - y) * (yk - 0.5f - y) - r2 <= 0)
		{
			SetPixel(xk, yk, pwColor);
		}
		else
		{
			yk -= 1.f;
			SetPixel(xk, yk, pwColor);
		}
		xk += 1.f;
	}
	while (yk >= y)
	{
		if ((xk - x + 0.5f)* (xk - x + 0.5f) + (yk - y) * (yk - y) - r2 >= 0)
		{
			SetPixel(xk, yk, pwColor);
		}
		else
		{
			xk += 1.f;
			SetPixel(xk, yk, pwColor);
		}
		yk -= 1.f;
	}
}

void PW_3DDevice::AddLight(PW_Light* pLight)
{
	m_vLights.push_back(pLight);
	m_bUseLight = true;
}

void PW_3DDevice::UpdateCurLight()
{
	for (int i = 0; i < m_vLights.size(); ++i)
	{
		PW_Vector3D lightdir;
		if (m_vLights[i]->m_iLightType == pw_lt_spotlight)
		{
		}
		else if (m_vLights[i]->m_iLightType == pw_lt_pointlight)
		{
			RenderShadowMap(m_vLights[i]);
			PW_Vector3D vP = m_vLights[i]->m_vPosition.MatrixProduct(m_pCamera->GetViewMat());
			lightdir = vP;
			m_vLights[i]->m_vCurDir = vP;
			//lightdir.Normalize();
		}
		else if (m_vLights[i]->m_iLightType == pw_lt_arealight)
		{
			PW_Vector3D vP = m_vLights[i]->m_vPosition.MatrixProduct(m_pCamera->GetViewMat());
			lightdir = vP;
			m_vLights[i]->m_vCurDir = vP;
		}
		else
		{
			RenderShadowMap(m_vLights[i]);
			PW_Vector3D vOri;
			vOri = vOri.MatrixProduct(m_pCamera->GetViewMat());
			PW_Vector3D vP = m_vLights[i]->m_vDirection.MatrixProduct(m_pCamera->GetViewMat());
			lightdir = vOri - vP;
			lightdir.Normalize();
			m_vLights[i]->m_vCurDir = lightdir;
		}
	}
}

PW_COLORF PW_3DDevice::RayComputerLight(PW_RayTraceNode* pNode)
{
	if (!pNode)
	{
		return PW_COLORF(0,0);
	}
	if (abs(pNode->Light.vDir.x) < EPSILON &&
		abs(pNode->Light.vDir.y) < EPSILON &&
		abs(pNode->Light.vDir.z) < EPSILON)
	{
		return PW_COLORF(0, 0);
	}
	if (m_pMeshs[pNode->nMeshIndex]->material.bEmissive)//自发光
	{
		return m_pMeshs[pNode->nMeshIndex]->material.cEmission;
	}
	PW_COLORF cAmbient, cDiffuse, cSpecular, cEmissive;
	PW_COLORF cP, cD, cS, cE;
	cP = 0;
	cD = 0;
	cS = 0;
	PW_FLOAT fSpe = 0;
	PW_COLORF cRet;//光强
#define RAYTRACELIGHT
	for (int i = 0; i < m_vLights.size(); ++i)
	{
		if (!m_vLights[i]->m_bOn)
			continue;
#ifdef RAYTRACELIGHT
		cRet += m_vLights[i]->RayTraceColor(pNode->Light.vStart, pNode->Light ,pNode->nMeshIndex, pNode->Light.vNormal, pNode->Light.vOriDir);
#else

		
		PW_FLOAT par = 1.0f;
		PW_Vector3D lightdir;
		if (m_vLights[i]->m_iLightType == pw_lt_spotlight)
		{
		}
		else if (m_vLights[i]->m_iLightType == pw_lt_pointlight)
		{
			PW_Vector3D vP = m_vLights[i]->m_vCurDir;
			lightdir = vP - pNode->Light.vStart;
			lightdir.Normalize();
		}
		else if (m_vLights[i]->m_iLightType == pw_lt_arealight)
		{
			PW_Vector3D vP = m_vLights[i]->m_vCurDir;
			lightdir = vP - pNode->Light.vStart;
			lightdir.Normalize();
		}
		else
		{
			lightdir = m_vLights[i]->m_vCurDir;
		}
		
		PW_BOOL bHasObjIn = PW_FALSE;
		for (int ii = 0; ii < m_pMeshs.size(); ii++)
		{
			if (ii != pNode->nMeshIndex &&!m_pMeshs[ii]->material.bEmissive && m_pMeshs[ii]->RayInsertion(pNode->Light.vStart, lightdir))
			{
				bHasObjIn = PW_TRUE;
				break;
			}
		}
		if (!bHasObjIn)
		{
			PW_FLOAT fRes = PW_DotProduct(lightdir, pNode->Light.vNormal);
			if (fRes > EPSILON)
			{
				cD = cD + m_vLights[i]->GetDiffuse(&pNode->Light.vStart) * fRes;
			}

			//相机总是在0,0,0
			PW_Vector3D vP = pNode->Light.vOriDir * -1;
			vP.Normalize();
			vP = vP + lightdir;
			vP.Normalize();
			PW_FLOAT fTmp = PW_DotProduct(pNode->Light.vNormal, vP);
			fTmp = pow(fTmp, m_pMeshs[pNode->nMeshIndex]->material.fP);
			if (fTmp > 0)
			{
				cS = cS + m_vLights[i]->GetSpecular(&pNode->Light.vStart) * fTmp;
			}
		}


		cP = cP + m_vLights[i]->m_cAmbient;
#endif // RAYTRACELIGHT
	}
#ifndef RAYTRACELIGHT
	cAmbient = pNode->Light.cAmbient * cP;
	cDiffuse = pNode->Light.cDiffuse * cD;
	cSpecular = pNode->Light.cSpecularReflection * cS;

	cRet = cAmbient + cDiffuse + cSpecular;
#endif // RAYTRACELIGHT



	return cRet;
}

PW_COLORF PW_3DDevice::RayTraceRec(PW_RayTraceNode* pNode, PW_INT nDepth, PW_INT& nOutTotalD, PW_FLOAT& dis)
{
	PW_COLORF retColorf;
	if (!pNode)
	{
		return m_Ambient;
	}
	if (nDepth >= m_nMaxDepth)
	{
		return m_Ambient;
	}
	if (pNode->bDrawPath)
	{
		PW_Vector4D vSS = pNode->Light.vStart.MatrixProduct(m_pCamera->GetProjMat());
		PW_Vector4D vEE = (pNode->Light.vDir * 10.f + pNode->Light.vStart).MatrixProduct(m_pCamera->GetProjMat());
		vEE.NoneHomogeneous();
		vSS.NoneHomogeneous();
		vSS.MatrixProduct(m_viewportMatrix);
		vEE.MatrixProduct(m_viewportMatrix);
		//m_vecPath.push_back(PW_Vertex(vSS, PW_RGB(255, 255, 255)));
		//m_vecPath.push_back(PW_Vertex(vEE, PW_RGB(255, 255, 255)));

	}


	PW_BOOL bInsert = PW_FALSE;
	dis = 10000000.f;
	PW_LightRay r1, r2;
	PW_INT nSele = -1;
	PW_INT nRes = -1;
	PW_INT nLastRes = -1;
	PW_LightRay pwlight1,pwlight2;
	for (int i = 0; i < m_pMeshs.size();i++)
	{
		nRes = m_pMeshs[i]->RayReflect(pNode->Light, r1, r2/*, pNode->bDrawPath*/);
		pNode->bInsert = PW_FALSE;
		
		if (nRes > 0)
		{
			PW_FLOAT fThisLen = (r1.vStart - pNode->Light.vStart).GetLen();
			if (fThisLen > dis || fThisLen  < EPSILON * 1000.f)
			{
				continue;
			}
			else
				dis = fThisLen;

			bInsert = PW_TRUE;
			nSele = i;
			pwlight1 = r1;
			pwlight2 = r2;
			nLastRes = nRes;
		}
	}
	if (bInsert)
	{
		r1 = pwlight1;
		r2 = pwlight2;
		pNode->bInsert = PW_TRUE;
		PW_RayTraceNode* pL = &g_Node[m_nCurNodePos++];
		pL->nMeshIndex = nSele;
		pL->pRight = NULL;
		pL->pLeft = NULL;
		pL->Light = r1;
		pL->bDrawPath = pNode->bDrawPath;
		pNode->pLeft = pL;
		nOutTotalD++;
		PW_COLORF fT = RayComputerLight(pL);

		PW_FLOAT fLen = 10000000.f;
		PW_COLORF fL;
		if (pNode->bDrawPath)
		{
			PW_Vector4D vSS = pNode->Light.vStart.MatrixProduct(m_pCamera->GetProjMat());
			PW_Vector4D vEE = pL->Light.vStart.MatrixProduct(m_pCamera->GetProjMat());
			vEE.NoneHomogeneous();
			vSS.NoneHomogeneous();
			vSS.MatrixProduct(m_viewportMatrix);
			vEE.MatrixProduct(m_viewportMatrix);
			m_vecPath.push_back(PW_Vertex(vSS, PW_RGB(255, 255, 255)));
			m_vecPath.push_back(PW_Vertex(vEE, PW_RGB(255, 255, 255)));
			//DrawLine2D(PW_Vertex(vSS, PW_RGB(255, 255, 255)), PW_Vertex(vEE, PW_RGB(255, 255,255)), 1, 0);
		}


		if (!m_pMeshs[nSele]->material.bEmissive)
			fL = RayTraceRec(pL, nDepth + 1, nOutTotalD, fLen);

		PW_FLOAT fRate = 0.8f;
		//if (fRate > 1.f)
		//{
		//	fRate = 0.5f;
		//}
		//else
		//{
		//	fRate *= 0.8f;
		//}
		//if (fRate < 1.f && fLen <10000000.f)
		{
			fL = fL * fRate;
		}

		PW_COLORF fR;
		if (nLastRes > 1 && !m_pMeshs[nSele]->material.bEmissive)
		{
			PW_RayTraceNode* pR = &g_Node[m_nCurNodePos++];
			//memset(pL, 0, sizeof(PW_RayTraceNode));
			pR->Light = r2;
			pR->pLeft = NULL;
			pR->pRight = NULL;
			pNode->pRight = pR;
			pR->bDrawPath = pNode->bDrawPath;
			fR = RayTraceRec(pR, nDepth + 1, nOutTotalD, fLen);
			fRate = 0.8f;//0.8g(30.f / (pow(PW_FLOAT(nDepth + 1), 0)* (fLen + 1.f)));
			//if (fRate < 1.f)
			{
				//fL *= fRate;
				fR = fR * fRate;
			}
			if (pNode->bDrawPath)
			{
				PW_Vector4D vSS = pNode->Light.vStart.MatrixProduct(m_pCamera->GetProjMat());
				PW_Vector4D vEE = pR->Light.vStart.MatrixProduct(m_pCamera->GetProjMat());
				vEE.NoneHomogeneous();
				vSS.NoneHomogeneous();
				vSS.MatrixProduct(m_viewportMatrix);
				vEE.MatrixProduct(m_viewportMatrix);
				m_vecPath.push_back(PW_Vertex(vSS, PW_RGB(255, 0, 0)));
				m_vecPath.push_back(PW_Vertex(vEE, PW_RGB(255, 0, 0)));
				//	DrawLine2D(PW_Vertex(vSS, PW_RGB(255, 255, 255)), PW_Vertex(vEE, PW_RGB(255, 255, 255)), 1, 0);
			}
		}
		retColorf = retColorf + fL * pL->Light.cSpecularReflection/*m_pMeshs[nSele]->material.cSpecularReflection*/ + fR /** m_pMeshs[nSele]->material.cDiffuse*/ + fT;
	}
	else
		return m_Ambient;
	return retColorf;
}

void PW_3DDevice::RenderShadowMap(PW_Light* pLight)
{
	if (!pLight)
	{
		return;
	}
	fnPixelShader = ShadowMapPS;
	fnVertexShader = ShadowMapVS;
	m_dwRenderState = PW_RS_ENABLEZWRITE | PW_RS_ENABLEZTEST /*| PW_RS_ENABLECOLORWRITE*/;
	PW_FLOAT* pOldBuffer = GetZBuffer();
	PW_CameraBase* pOldCamera = m_pCamera;
	m_pCamera = m_pShadowMapCamera;
	SetZBuffer(m_pShadowZBuffer);

	PW_Vector3D vMin(10000.f, 10000.f, 10000.f);
	PW_Vector3D vMax(-10000.f, -10000.f, -10000.f);
	

	for (int n = 0; n < m_pMeshs.size(); n++)
	{
		if (m_pMeshs[n]->bCastShadow)
		{
			PW_Vector3D pwOri;
			SetMaterial(&m_pMeshs[n]->material);
			pwOri = pwOri.MatrixProduct(m_pMeshs[n]->m_absoluteTM);

			for (int i = 0; i < m_pMeshs[n]->pointcount; ++i)
			{
				m_v4dBuffer[i] = m_pMeshs[n]->buffer[i].MatrixProduct(m_pMeshs[n]->m_absoluteTM);

				vMin.x = fmin(m_v4dBuffer[i].x, vMin.x);
				vMin.y = fmin(m_v4dBuffer[i].y, vMin.y);
				vMin.z = fmin(m_v4dBuffer[i].z, vMin.z);

				vMax.x = fmax(m_v4dBuffer[i].x, vMax.x);
				vMax.y = fmax(m_v4dBuffer[i].y, vMax.y);
				vMax.z = fmax(m_v4dBuffer[i].z, vMax.z);
			}
			//for (int i = 0; i < m_pMeshs[n]->indexcount; i++)
			//{
			//	int index1 = m_pMeshs[n]->indexbuffer[i][0];
			//	int index2 = m_pMeshs[n]->indexbuffer[i][1];
			//	int index3 = m_pMeshs[n]->indexbuffer[i][2];
			//	DrawTriPrimitive(PW_POINT3D(m_v4dBuffer[index1], m_pMeshs[n]->buffer[index1].pwColor, m_vNormalsBuffer[index1], m_pMeshs[n]->indexbuffer[i].u1, m_pMeshs[n]->indexbuffer[i].v1)
			//		, PW_POINT3D(m_v4dBuffer[index2], m_pMeshs[n]->buffer[index2].pwColor, m_vNormalsBuffer[index2], m_pMeshs[n]->indexbuffer[i].u2, m_pMeshs[n]->indexbuffer[i].v2)
			//		, PW_POINT3D(m_v4dBuffer[index3], m_pMeshs[n]->buffer[index3].pwColor, m_vNormalsBuffer[index3], m_pMeshs[n]->indexbuffer[i].u3, m_pMeshs[n]->indexbuffer[i].v3), PW_RGBA(255, 0, 0), m_ds);

			//}
		}
		
	}
	PW_Vector3D vLookAt;
	vLookAt.x = (vMin.x + vMax.x) / 2.f;
	vLookAt.y = (vMin.x + vMax.y) / 2.f;
	vLookAt.z = (vMin.z + vMax.z) / 2.f;
	PW_Vector3D vLen = vMax - vMin;
	PW_FLOAT fExtend = vLen.GetLen() / 2.f;
	if (pLight->m_iLightType == pw_lt_pointlight)
	{
		
		PW_Vector3D vPos =  pLight->GetPos();
		PW_Vector3D vDir = vLookAt - vPos;
		PW_FLOAT fFov = atan(fExtend / vDir.GetLen());
		vDir.Normalize();
		PW_Vector3D vUp = vDir + PW_Vector3D(vDir.x + 1, vDir.y * 2 + 2, vDir.z * 3 + 3);

		m_pShadowMapCamera->SetCamerInfo(vPos, vLookAt, vUp, fFov, 1.f, 1.f, 10000.f);
	}
	else if (pLight->m_iLightType == pw_lt_directionallight)
	{
		
		PW_Vector3D vDir = pLight->m_vDirection;
		vDir.Normalize();

		PW_Vector3D vPos = vLookAt - vDir * (fExtend + 5.f);

		PW_Vector3D vUp = vDir + PW_Vector3D(vDir.x + 1, vDir.y * 2 + 2, vDir.z * 3 + 3);

		m_pShadowMapCamera->SetCamerInfo(vPos, vLookAt, vUp, fExtend, 1.f, 1.f, 10000.f);
	}
	

	for (int n = 0; n < m_pMeshs.size(); n++)
	{
		if (m_pMeshs[n]->bCastShadow)
		{
			PW_Vector3D pwOri;
			SetMaterial(&m_pMeshs[n]->material);
			pwOri = pwOri.MatrixProduct(m_pMeshs[n]->m_absoluteTM);

			for (int i = 0; i < m_pMeshs[n]->pointcount; ++i)
			{
				m_v4dBuffer[i] = m_pMeshs[n]->buffer[i].MatrixProduct(m_pMeshs[n]->m_absoluteTM);
			}
			for (int i = 0; i < m_pMeshs[n]->indexcount; i++)
			{
				int index1 = m_pMeshs[n]->indexbuffer[i][0];
				int index2 = m_pMeshs[n]->indexbuffer[i][1];
				int index3 = m_pMeshs[n]->indexbuffer[i][2];
				DrawTriPrimitive(PW_POINT3D(m_v4dBuffer[index1], m_pMeshs[n]->buffer[index1].pwColor, m_vNormalsBuffer[index1], m_pMeshs[n]->indexbuffer[i].u1, m_pMeshs[n]->indexbuffer[i].v1)
					, PW_POINT3D(m_v4dBuffer[index2], m_pMeshs[n]->buffer[index2].pwColor, m_vNormalsBuffer[index2], m_pMeshs[n]->indexbuffer[i].u2, m_pMeshs[n]->indexbuffer[i].v2)
					, PW_POINT3D(m_v4dBuffer[index3], m_pMeshs[n]->buffer[index3].pwColor, m_vNormalsBuffer[index3], m_pMeshs[n]->indexbuffer[i].u3, m_pMeshs[n]->indexbuffer[i].v3), PW_RGBA(255, 0, 0), m_ds);

			}
		}

	}
	SetZBuffer(pOldBuffer);
	m_pCamera = pOldCamera;
}

void PW_3DDevice::RenderScene()
{
	if (!m_v4dBuffer)
	{
		return;
	}
	UpdateCurLight();


	fnVertexShader = VertexShader;
	m_dwRenderState = PW_RS_ENABLEZWRITE | PW_RS_ENABLEZTEST | PW_RS_ENABLECOLORWRITE;
	for (int n = 0; n < m_pMeshs.size(); n++)
	{
		if (m_pMeshs[n]->bReceiveShadow)
		{
			fnPixelShader = ReceiveShadowPS;
		}
		else
			fnPixelShader = PixelShader;
		PW_Vector3D pwOri;
		SetMaterial(&m_pMeshs[n]->material);
		pwOri = pwOri.MatrixProduct(m_pMeshs[n]->m_absoluteTM);

		for (int i = 0; i < m_pMeshs[n]->pointcount; ++i)
		{
			m_v4dBuffer[i] = m_pMeshs[n]->buffer[i].MatrixProduct(m_pMeshs[n]->m_absoluteTM);
			m_vNormalsBuffer[i] = m_pMeshs[n]->buffer[i].vNormal.MatrixProduct(m_pMeshs[n]->m_absoluteTM);
			m_v4dBuffer[i].pwColor = m_pMeshs[n]->buffer[i].pwColor;

			m_vNormalsBuffer[i] = m_vNormalsBuffer[i] - pwOri;
			m_vNormalsBuffer[i].Normalize();

		}
		for (int i = 0; i < m_pMeshs[n]->indexcount; i++)
		{
			int index1 = m_pMeshs[n]->indexbuffer[i][0];
			int index2 = m_pMeshs[n]->indexbuffer[i][1];
			int index3 = m_pMeshs[n]->indexbuffer[i][2];
			DrawTriPrimitive(PW_POINT3D(m_v4dBuffer[index1], m_pMeshs[n]->buffer[index1].pwColor, m_vNormalsBuffer[index1], m_pMeshs[n]->indexbuffer[i].u1, m_pMeshs[n]->indexbuffer[i].v1)
				, PW_POINT3D(m_v4dBuffer[index2], m_pMeshs[n]->buffer[index2].pwColor, m_vNormalsBuffer[index2], m_pMeshs[n]->indexbuffer[i].u2, m_pMeshs[n]->indexbuffer[i].v2)
				, PW_POINT3D(m_v4dBuffer[index3], m_pMeshs[n]->buffer[index3].pwColor, m_vNormalsBuffer[index3], m_pMeshs[n]->indexbuffer[i].u3, m_pMeshs[n]->indexbuffer[i].v3), PW_RGBA(255, 0, 0), m_ds);

		}
	}

}

void PW_3DDevice::RayTrace()
{
	m_vecPath.clear();
	for (int i = 0; i < m_pMeshs.size();i++)
	{
		m_pMeshs[i]->ComputeCurVertex();
	}
	UpdateCurLight();
	PW_Vector3D raystart, rayend;
	PW_INT nOutD = 0;
	PW_FLOAT flen;
	for (int x = 0; x <  this->m_iWidth; x++)
	{
		for (int y = 0; y < this->m_iHeight; y++)
		{
			m_nCurNodePos = 0;
			raystart.x = x;
			raystart.y = y;
			raystart.z = 0.f;
			rayend.x = x;
			rayend.y = y;
			rayend.z = 1.f;

			GetViewPos(raystart, raystart);
			GetViewPos(rayend, rayend);
			PW_RayTraceNode*Root = &g_Node[m_nCurNodePos++];
			
			Root->pLeft = NULL;
			Root->pRight = NULL;
			Root->Light.vStart = raystart;
			Root->Light.vDir = raystart;//rayend - raystart;
			Root->Light.vDir.Normalize();
			if (m_nDrawX == x && m_nDrawY == y)
			{
				Root->bDrawPath = PW_TRUE;
			}
			else
			{
				Root->bDrawPath = PW_FALSE;
			}
			nOutD = 0;
			PW_COLORF fr = RayTraceRec(Root, 0, nOutD, flen);//RayComputerLight(Root);
		
			PW_COLOR pwc = fr * PW_COLOR(PW_RGB(255, 255, 255));

			SetPixel(x, y, pwc);
		}
	}
	for (int i = 0; i < m_vecPath.size();i += 2)
	{
		DrawLine2D(m_vecPath[i], m_vecPath[i + 1], 1, 0);
	}
}