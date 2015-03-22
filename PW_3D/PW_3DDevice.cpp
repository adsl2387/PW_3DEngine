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
	m_hWnd = NULL;
	m_hBitmap = NULL;
	m_ds = wireframe;
	m_bUseMaterial = false;
	m_bUseLight = false;
	m_bUseBiliner = false;
	m_bRayTrace = false;
}

PW_3DDevice::~PW_3DDevice()
{
	this->Release();
}



bool PW_3DDevice::Create(HWND hWnd, int iWidth, int iHeight, HWND hEdit)
{
	m_nMaxDepth = 2;
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
	}
	else
		this->m_pMeshs.clear();
	
	QuadMemSet(m_pBitBuffer, m_iWidth * m_iHeight * sizeof(DWORD), pwcolor);
	m_fMaxZ = 0;
	m_fMinZ = 1;
	m_curV4DPos = 0;
	m_curIndexPos = 0;
	m_nCurNodePos = 0;
	//memset(g_Node, 0, sizeof(g_Node));
}

void PW_3DDevice::Present()
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
	TextOut(hDc, 0, 0, buffer, strlen(buffer));
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
}

void PW_3DDevice::Render()
{
	
	if (m_bRayTrace)
	{
		RayTrace();
	}
	//else
	//{
	Update();
	//}

}

void PW_3DDevice::Update()
{
	Present();
	Clear(PW_RGBA(127, 127, 127), 1.f);
}

void PW_3DDevice::DrawLineTexture(PW_POINT3D point1, PW_POINT3D point2, int isolid /* = 1 */)
{
	if (!m_pBitBuffer)
	{
		return;
	}
	m_bShow++;

	int dx = ROUND(point2.x) - ROUND(point1.x);
	int dy = ROUND(point2.y) - ROUND(point1.y);
	PW_FLOAT fvz1 = 1.f / GetViewZ(point1.z);
	PW_FLOAT fvz2 = 1.f / GetViewZ(point2.z);
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
		if (GetValueOfZBuffer(ROUND(point1.x), ROUND(point1.y)) - point1.z> 0)
		{
			PW_COLOR pwColor;
			if (m_bUseBiliner)
			{
				pwColor = point1.fP * m_texture->BiLinerGetColor(point1.u, point1.v);
			}
			else
				pwColor = point1.fP * m_texture->GetColor(point1.u, point1.v);
			SetValueOfCBuffer(ROUND(point1.x), ROUND(point1.y), pwColor);
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
	PW_COLORF fIncrementlp1 = (point2.fP * fvz2 - point1.fP * fvz1) / PW_FLOAT(steps);

	PW_FLOAT fIncrementu1 = (point2.u * fvz2 - point1.u * fvz1) / PW_FLOAT(steps);
	PW_FLOAT fIncrementv1 = (point2.v * fvz2 - point1.v * fvz1) / PW_FLOAT(steps);


	PW_FLOAT fx = point1.x;
	PW_FLOAT fy = point1.y;
	PW_FLOAT fz =  fvz1;

	PW_COLORF fLp1 = point1.fP * fvz1;

	PW_FLOAT fU1 = point1.u * fvz1;
	PW_FLOAT fV1 = point1.v * fvz1;

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
		PW_FLOAT fsz = GetViewPortZ(1.f / fz);
		if (GetValueOfZBuffer(ROUND(fx), ROUND(fy)) - fsz > 0/*m_pZBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] - fz> 0*/)
		{
			PW_COLOR pwColor;
			PW_FLOAT fsu,fsv;
			PW_FLOAT fdz = 1.f / fz;
			fsu = fU1 * fdz;
			fsv = fV1 * fdz;
			if (m_bUseBiliner)
			{
				pwColor = fLp1 * fdz * m_texture->BiLinerGetColor(fsu, fsv);
			}
			else
				pwColor = fLp1 * fdz * m_texture->GetColor(fsu, fsv);
		
			SetValueOfCBuffer(ROUND(fx), ROUND(fy), pwColor);
			SetValueOfZBuffer(ROUND(fx), ROUND(fy), fsz);
		}

	}
}


void PW_3DDevice::DrawLine3D(PW_POINT3D point1, PW_POINT3D point2)
{

}

void PW_3DDevice::DrawLine2D(PW_POINT3D point1, PW_POINT3D point2, int isolid)
{
	if (!m_pBitBuffer)
	{
		return;
	}

	point1.y = m_iHeight - point1.y;
	point2.y = m_iHeight - point2.y;
	int dx = ROUND(point2.x) - ROUND(point1.x);
	int dy = ROUND(point2.y) - ROUND(point1.y);
	PW_FLOAT fvz1 = 1.f / GetViewZ(point1.z);
	PW_FLOAT fvz2 = 1.f / GetViewZ(point2.z);
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
		if (m_pZBuffer[ROUND(point1.y) * m_iWidth + ROUND(point1.x)] - point1.z> 0)
		{
			PW_COLOR pwColor = point1.pwColor;
			if (m_bUseTexture)
			{
				pwColor = point1.fP * m_texture->GetColor(point1.u, point1.v);
			}
			m_pBitBuffer[ROUND(point1.y) * m_iWidth + ROUND(point1.x)] = pwColor;
			m_pZBuffer[ROUND(point1.y) * m_iWidth + ROUND(point1.x)] = point1.z;

		}	
	}
	if (steps == 0)
	{
		return;
	}
	PW_FLOAT fIncrementx = dx / (PW_FLOAT)steps;
	PW_FLOAT fIncrementy = dy / (PW_FLOAT)steps;
	PW_FLOAT fIncrementz = dz / steps;
	PW_FLOAT fIncrementr = ((PW_FLOAT)(PW_RGBA_R(point2.pwColor)) * fvz2 - PW_FLOAT(PW_RGBA_R(point1.pwColor)) * fvz1) / PW_FLOAT(steps);
	PW_FLOAT fIncrementg = ((PW_FLOAT)(PW_RGBA_G(point2.pwColor)) * fvz2 - PW_FLOAT(PW_RGBA_G(point1.pwColor)) * fvz1) / PW_FLOAT(steps);
	PW_FLOAT fIncrementb = ((PW_FLOAT)(PW_RGBA_B(point2.pwColor)) * fvz2 - PW_FLOAT(PW_RGBA_B(point1.pwColor)) * fvz1) / PW_FLOAT(steps);

	PW_FLOAT fx = point1.x;
	PW_FLOAT fy = point1.y;
	PW_FLOAT fz = fvz1;
	PW_FLOAT fr = (PW_FLOAT)(PW_RGBA_R(point1.pwColor)) * fvz1;
	PW_FLOAT fg = (PW_FLOAT)(PW_RGBA_G(point1.pwColor)) * fvz1;
	PW_FLOAT fb = PW_FLOAT(PW_RGBA_B(point1.pwColor)) * fvz1;

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
		PW_FLOAT fdz = 1.f / fz;
		PW_FLOAT fsz = GetViewPortZ(fdz);
		if (m_pZBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] - fsz> 0)
		{	
			PW_COLOR pwColor =  PW_RGBA(ROUND(fr * fdz), ROUND(fg * fdz), ROUND(fb * fdz));
			m_pBitBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] = pwColor;
			m_pZBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] = fsz;
		}
	
	}
}

void PW_3DDevice::DrawMesh(PW_Mesh& mesh)
{
	m_bShow = 0;
	PW_Matrix4D tran;
	PW_Matrix4D tran1;
	PW_MatrixProduct4D(m_pCamera->GetViewMat(), mesh.m_matAbsTM, tran);

	if (m_bRayTrace)
	{
		//PW_Mesh me = mesh;
		mesh.m_absoluteTM = tran;
		m_pMeshs.push_back(&mesh);
		return;
	}
	if (!m_v4dBuffer)
	{
		return;
	}
	for (int i = 0; i < mesh.pointcount;++i)
	{
		m_v4dBuffer[i] = mesh.buffer[i].MatrixProduct(tran);
		m_vNormalsBuffer[i] = mesh.buffer[i].vNormal.MatrixProduct(tran);
		m_v4dBuffer[i].pwColor = mesh.buffer[i].pwColor;
		PW_Vector3D pwOri;
		pwOri = pwOri.MatrixProduct(tran);
		m_vNormalsBuffer[i] = m_vNormalsBuffer[i] - pwOri;
		m_vNormalsBuffer[i].Normalize();
		
	}
	for (int i = 0; i < mesh.indexcount;i++)
	{
		int index1 = mesh.indexbuffer[i][0];
		int index2 = mesh.indexbuffer[i][1];
		int index3 = mesh.indexbuffer[i][2];
		DrawTriPrimitive(PW_POINT3D(m_v4dBuffer[index1], mesh.buffer[index1].pwColor, m_vNormalsBuffer[index1], mesh.indexbuffer[i].u1, mesh.indexbuffer[i].v1)
			, PW_POINT3D(m_v4dBuffer[index2], mesh.buffer[index2].pwColor, m_vNormalsBuffer[index2], mesh.indexbuffer[i].u2, mesh.indexbuffer[i].v2)
			, PW_POINT3D(m_v4dBuffer[index3], mesh.buffer[index3].pwColor, m_vNormalsBuffer[index3], mesh.indexbuffer[i].u3, mesh.indexbuffer[i].v3), PW_RGBA(255, 0, 0), m_ds);
	
	}
	//for (int i = 0; i < mesh.pointcount;i++)
	//{
	//	PW_Vector3D start = m_v4dBuffer[i];
	//	PW_Vector3D end = start + m_vNormalsBuffer[i] * 50;
	//	PW_Vector4D p1, p2, p3;
	//	PW_Matrix4D tan;
	//	p1 = start.MatrixProduct(m_projMatrix);
	//	p2 = end.MatrixProduct(m_projMatrix);
	//	//p3 = point3.MatrixProduct(m_projMatrix);
	//	p1.NoneHomogeneous();
	//	p2.NoneHomogeneous();
	//	//p3.NoneHomogeneous();
	//	p1.MatrixProduct(m_viewportMatrix);
	//	p2.MatrixProduct(m_viewportMatrix);
	////	p3.MatrixProduct(m_viewportMatrix);
	//	DrawLine(PW_POINT3D(p1, PW_RGBA(200, 200, 200)), PW_POINT3D(p2, PW_RGBA(200, 200, 200)));
	//}
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
			PW_FLOAT par = 1.0f;
			PW_Vector3D lightdir;
			if (m_vLights[i]->m_iLightType == pw_lt_spotlight)
			{
			}
			else if (m_vLights[i]->m_iLightType == pw_lt_pointlight)
			{
				PW_Vector3D vP = m_vLights[i]->m_vPosition.MatrixProduct(viewMat);
				lightdir = vP - point;
			}
			else
			{
				PW_Vector3D vOri;
				vOri = vOri.MatrixProduct(viewMat);
				PW_Vector3D vP = m_vLights[i]->m_vDirection.MatrixProduct(viewMat);
				lightdir = vOri - vP;
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

//观察坐标系中的点
void PW_3DDevice::DrawTriPrimitive(PW_POINT3D point1, PW_POINT3D point2, PW_POINT3D point3, PW_COLOR color, int ds)
{
	PW_Vector3D v = point2 - point1;
	PW_Vector3D u = point3 - point2;
	PW_Vector3D dir(0, 0, 0);
	dir = dir - point1;
	PW_Vector3D h;
	PW_CrossProduct(v, u, h);
	PW_FLOAT dotRes = PW_DotProduct(h, dir);

	//消除背面
	if (dotRes <= 0)
	{
		return;
	}
	PW_Vector4D p1, p2, p3;
	PW_Matrix4D tan, projMatrix;
	projMatrix = this->m_pCamera->GetProjMat();
	p1 = point1.MatrixProduct(projMatrix);
	p2 = point2.MatrixProduct(projMatrix);
	p3 = point3.MatrixProduct(projMatrix);
	p1.NoneHomogeneous();
	p2.NoneHomogeneous();
	p3.NoneHomogeneous();
	p1.MatrixProduct(m_viewportMatrix);
	p2.MatrixProduct(m_viewportMatrix);
	p3.MatrixProduct(m_viewportMatrix);
	
	//计算光强
	ComputeLight(point1, m_pCamera->GetViewMat());
	ComputeLight(point2, m_pCamera->GetViewMat());
	ComputeLight(point3, m_pCamera->GetViewMat());
	
	if (ds == wireframe)
	{
		//if (dotRes > 0)
		{
			DrawLine2D(PW_POINT3D(p1, point1.pwColor, point1.vNormal, point1.u, point1.v, point1.fP)
				, PW_POINT3D(p2, point2.pwColor, point2.vNormal, point2.u, point2.v, point2.fP));
			DrawLine2D(PW_POINT3D(p2, point2.pwColor, point2.vNormal, point2.u, point2.v, point2.fP)
				, PW_POINT3D(p3, point3.pwColor, point3.vNormal, point3.u, point3.v, point3.fP));
			DrawLine2D(PW_POINT3D(p3, point3.pwColor, point3.vNormal, point3.u, point3.v, point3.fP)
				, PW_POINT3D(p1, point1.pwColor, point1.vNormal, point1.u, point1.v, point1.fP));
		}
	}
	else
	{
		//if (dotRes > 0)
		{
			DrawTriangle(PW_POINT3D(p1, point1.pwColor, PW_Vector3D(), point1.u, point1.v, point1.fP)
				, PW_POINT3D(p2, point2.pwColor, PW_Vector3D(), point2.u, point2.v, point2.fP)
				, PW_POINT3D(p3, point3.pwColor, PW_Vector3D(), point3.u, point3.v, point3.fP));
		}
		
	}
	
}

void PW_3DDevice::DrawTriangle(PW_POINT3D point1, PW_POINT3D point2, PW_POINT3D point3)
{

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

	int cury = ROUND(pps[0].y);
	PW_POINT3D leftPoint, rightPoint;
	PW_FLOAT fZl,fZr;
	if (dy1 > 0)
	{
		PW_FLOAT fIncrementx1 =(pps[1].x - pps[0].x) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementx2 = (pps[2].x - pps[0].x) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementz1 = (1.0f / fZ1 - 1.0f / fZ) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementz2 = (1.0f /fZ2 - 1.0f / fZ) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementr1 = ((PW_FLOAT)PW_RGBA_R(pps[1].pwColor) / fZ1 - (PW_FLOAT)PW_RGBA_R(pps[0].pwColor) / fZ) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementg1 = ((PW_FLOAT)PW_RGBA_G(pps[1].pwColor) / fZ1- (PW_FLOAT)PW_RGBA_G(pps[0].pwColor) / fZ) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementb1 = ((PW_FLOAT)PW_RGBA_B(pps[1].pwColor) / fZ1- (PW_FLOAT)PW_RGBA_B(pps[0].pwColor) / fZ) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementr2 = ((PW_FLOAT)PW_RGBA_R(pps[2].pwColor) / fZ2- (PW_FLOAT)PW_RGBA_R(pps[0].pwColor) / fZ) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementg2 = ((PW_FLOAT)PW_RGBA_G(pps[2].pwColor) / fZ2- (PW_FLOAT)PW_RGBA_G(pps[0].pwColor) / fZ) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementb2 = ((PW_FLOAT)PW_RGBA_B(pps[2].pwColor) / fZ2- (PW_FLOAT)PW_RGBA_B(pps[0].pwColor) / fZ) / PW_FLOAT(dy2);
		PW_COLORF fIncrementlp1 = (pps[1].fP / fZ1 - pps[0].fP / fZ) / PW_FLOAT(dy1);
		PW_COLORF fIncrementlp2 = (pps[2].fP / fZ2 - pps[0].fP / fZ) / PW_FLOAT(dy2);
		PW_FLOAT fDu1 = (pps[1].u / fZ1 - pps[0].u / fZ) / PW_FLOAT(dy1);
		PW_FLOAT fDu2 = (pps[2].u / fZ2 - pps[0].u / fZ) / PW_FLOAT(dy2);
		PW_FLOAT fDv1 = (pps[1].v / fZ1 - pps[0].v / fZ) / PW_FLOAT(dy1);
		PW_FLOAT fDv2 = (pps[2].v / fZ2 - pps[0].v / fZ) / PW_FLOAT(dy2);
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
		fZl = 1.f / fZ;
		fZr = 1.f / fZ;
		
		PW_FLOAT fR1 = (PW_FLOAT)PW_RGBA_R(pps[0].pwColor) / fZ;
		PW_FLOAT fG1 = (PW_FLOAT)PW_RGBA_G(pps[0].pwColor) / fZ;
		PW_FLOAT fB1 = (PW_FLOAT)PW_RGBA_B(pps[0].pwColor) / fZ;
		PW_FLOAT fR2 = (PW_FLOAT)PW_RGBA_R(pps[0].pwColor) / fZ;
		PW_FLOAT fG2 = (PW_FLOAT)PW_RGBA_G(pps[0].pwColor) / fZ;
		PW_FLOAT fB2 = (PW_FLOAT)PW_RGBA_B(pps[0].pwColor) / fZ;
		PW_COLORF fLp1 = pps[0].fP / fZ;
		PW_COLORF fLp2 = pps[0].fP / fZ;
		PW_FLOAT fU1 = pps[0].u / fZ;
		PW_FLOAT fV1 = pps[0].v / fZ;
		PW_FLOAT fU2 = pps[0].u / fZ;
		PW_FLOAT fV2 = pps[0].v / fZ;
		int curY = ROUND(pps[0].y);

		if (m_bUseTexture)
		{
			DrawLineTexture(pps[0], pps[0]);
		}
		else
			DrawLine2D(pps[0], pps[0]);
		
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
				leftPoint.z = GetViewPortZ(1.f / fZl);
				leftPoint.fP = fLp1 / fZl;
				leftPoint.u = fU1 / fZl;
				leftPoint.v = fV1 / fZl;
				leftPoint.pwColor = PW_RGBA(ROUND(fR1 / fZl), ROUND(fG1 / fZl), ROUND(fB1 / fZl));
				rightPoint.x = fXr;
				rightPoint.y = curY;
				rightPoint.pwColor = PW_RGBA(ROUND(fR2 / fZr), ROUND(fG2 / fZr), ROUND(fB2 / fZr));
				rightPoint.z = GetViewPortZ(1.f / fZr);
				rightPoint.fP = fLp2 / fZr;
				rightPoint.u = fU2 / fZr;
				rightPoint.v = fV2 / fZr;
			}
			PW_POINT3D p1, p2;
			p1.x = fXl;
			p1.y = curY;
			p1.z = GetViewPortZ(1.f / fZl);
			p1.fP = fLp1 / fZl;
			p1.u = fU1 / fZl;
			p1.v = fV1 / fZl;
			p1.pwColor = PW_RGBA(ROUND(fR1 / fZl), ROUND(fG1 / fZl), ROUND(fB1 / fZl));
			p2.x = fXr;
			p2.y = curY;
			p2.z = GetViewPortZ(1.f / fZr);
			p2.pwColor = PW_RGBA(ROUND(fR2 / fZr), ROUND(fG2 / fZr), ROUND(fB2 / fZr));
			p2.fP = fLp2 / fZr;
			p2.u = fU2 / fZr;
			p2.v = fV2 / fZr;
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
			fZl = 1 / fZ;
			fZr = 1 / fZ1;
		}
		else
		{
			leftPoint = pps[1];
			rightPoint = pps[0];
			fZl = 1 / fZ1;
			fZr = 1 / fZ;
		}	
	}
	if (dy2 != 0 && dy2 > dy1)
	{
		dy1 = ROUND(pps[2].y) - ROUND(leftPoint.y);
		dy2 = ROUND(pps[2].y) - ROUND(rightPoint.y) ;
		PW_FLOAT fIncrementx1 = (pps[2].x - leftPoint.x) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementx2 = (pps[2].x - rightPoint.x) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementz1 = (1.f / fZ2 - fZl) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementz2 = (1.f / fZ2 - fZr) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementr1 = ((PW_FLOAT)PW_RGBA_R(pps[2].pwColor) / fZ2 - (PW_FLOAT)PW_RGBA_R(leftPoint.pwColor) * fZl) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementg1 = ((PW_FLOAT)PW_RGBA_G(pps[2].pwColor) / fZ2 - (PW_FLOAT)PW_RGBA_G(leftPoint.pwColor) * fZl) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementb1 = ((PW_FLOAT)PW_RGBA_B(pps[2].pwColor) / fZ2 - (PW_FLOAT)PW_RGBA_B(leftPoint.pwColor) * fZl) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementr2 = ((PW_FLOAT)PW_RGBA_R(pps[2].pwColor) / fZ2 - (PW_FLOAT)PW_RGBA_R(rightPoint.pwColor) * fZr) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementg2 = ((PW_FLOAT)PW_RGBA_G(pps[2].pwColor) / fZ2 - (PW_FLOAT)PW_RGBA_G(rightPoint.pwColor) * fZr) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementb2 = ((PW_FLOAT)PW_RGBA_B(pps[2].pwColor) / fZ2 - (PW_FLOAT)PW_RGBA_B(rightPoint.pwColor) * fZr) / PW_FLOAT(dy2);
		PW_COLORF fIncrementlp1 = (pps[2].fP / fZ2 - leftPoint.fP * fZl) / PW_FLOAT(dy1);
		PW_COLORF fIncrementlp2 = (pps[2].fP / fZ2 - rightPoint.fP * fZr) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementu1 = (pps[2].u / fZ2 - leftPoint.u * fZl) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementv1 = (pps[2].v / fZ2 - leftPoint.v * fZl) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementu2 = (pps[2].u / fZ2 - rightPoint.u * fZr) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementv2 = (pps[2].v / fZ2 - rightPoint.v * fZr) / PW_FLOAT(dy2);
		int steps = dy1;
		PW_FLOAT fCurR1 = (PW_FLOAT)PW_RGBA_R(leftPoint.pwColor) * fZl;
		PW_FLOAT fCurG1 = (PW_FLOAT)PW_RGBA_G(leftPoint.pwColor) * fZl;
		PW_FLOAT fCurB1 = (PW_FLOAT)PW_RGBA_B(leftPoint.pwColor) * fZl;
		PW_FLOAT fCurR2 = (PW_FLOAT)PW_RGBA_R(rightPoint.pwColor) * fZr;
		PW_FLOAT fCurG2 = (PW_FLOAT)PW_RGBA_G(rightPoint.pwColor) * fZr;
		PW_FLOAT fCurB2 = (PW_FLOAT)PW_RGBA_B(rightPoint.pwColor) * fZr;
		PW_FLOAT fx1 = (leftPoint.x);
		PW_FLOAT fx2 = (rightPoint.x);

		PW_COLORF flp1 = leftPoint.fP * fZl;
		PW_COLORF flp2 = rightPoint.fP * fZr;
		PW_FLOAT fU1 = leftPoint.u * fZl;
		PW_FLOAT fV1 = leftPoint.v * fZl;
		PW_FLOAT fU2 = rightPoint.u * fZr;
		PW_FLOAT fV2 = rightPoint.v * fZr;
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
			p1.z = GetViewPortZ(1.f / fZl);
			p1.fP = flp1 / fZl;
			p1.u = fU1 / fZl;
			p1.v = fV1 / fZl;

			p1.pwColor = PW_RGBA(ROUND(fCurR1 / fZl), ROUND(fCurG1 / fZl), ROUND(fCurB1 / fZl));
			p2.x = fx2;
			p2.y = curY;
			p2.z = GetViewPortZ(1.f / fZr);
			p2.fP = flp2 / fZr;
			p2.u = fU2 / fZr;
			p2.v = fV2 / fZr;
			p2.pwColor = PW_RGBA(ROUND(fCurR2 / fZr), ROUND(fCurG2 / fZr), ROUND(fCurB2 / fZr));
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
#ifdef RAYTRACELIGHT
		cRet += m_vLights[i]->RayTraceColor(pNode->Light.vStart, pNode->nMeshIndex, pNode->Light.vNormal, pNode->Light.vOriDir);
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

	PW_BOOL bInsert = PW_FALSE;
	dis = 10000000.f;
	PW_LightRay r1, r2;
	PW_INT nSele = -1;
	PW_INT nRes = -1;
	PW_LightRay pwlight1,pwlight2;
	for (int i = 0; i < m_pMeshs.size();i++)
	{
		nRes = m_pMeshs[i]->RayReflect(pNode->Light, r1, r2);
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
		pNode->pLeft = pL;
		nOutTotalD++;
		PW_COLORF fT = RayComputerLight(pL);

		PW_FLOAT fLen = 10000000.f;
		PW_COLORF fL;
		if (!m_pMeshs[nSele]->material.bEmissive)
			fL = RayTraceRec(pL, nDepth + 1, nOutTotalD, fLen);
		
		PW_FLOAT fRate = (500.f /  (fLen * fLen));
		if (fRate > 1.f)
		{
			fRate = 0.5f;
		}
		else
		{
			fRate *= 0.8f;
		}
		if (fRate < 1.f && fLen <10000000.f)
		{
			fL = fL * fRate;
		}

		PW_COLORF fR;
		if (nRes > 1 && !m_pMeshs[nSele]->material.bEmissive)
		{
			PW_RayTraceNode* pR = &g_Node[m_nCurNodePos++];
			//memset(pL, 0, sizeof(PW_RayTraceNode));
			pR->Light = r2;
			pR->pLeft = NULL;
			pR->pRight = NULL;
			pNode->pRight = pR;
			
			fR = RayTraceRec(pR, nDepth + 1, nOutTotalD, fLen);
			fRate = (30.f / (pow(PW_FLOAT(nDepth + 1), 0)* (fLen + 1.f)));
			if (fRate < 1.f)
			{
				//fL *= fRate;
				fR = fR * fRate;
			}

		}
		retColorf = retColorf + fL + fR + fT;
	}

	return retColorf;
}

void PW_3DDevice::RayTrace()
{
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
#ifdef AREATEST
			if (x == 396 && y == 334)
			{
				gBFlag = PW_TRUE;
			}
			else
			{
				gBFlag = PW_FALSE;
			}
#endif // AREATEST

			
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
			Root->Light.vDir = rayend - raystart;
			Root->Light.vDir.Normalize();
			nOutD = 0;
			PW_COLORF fr = RayTraceRec(Root, 0, nOutD, flen);//RayComputerLight(Root);
			if (nOutD > 0)
			{
				PW_COLOR pwc = fr * PW_COLOR(PW_RGB(255, 255, 255));

				SetPixel(x, y, pwc);
			}
		}
	}
}