#include "PW_3DDevice.h"
#include <stdlib.h>
#include <stdio.h>
PW_3DDevice::PW_3DDevice()
{
	m_pBitBuffer = NULL;
	m_pZBuffer = NULL;
	m_hWnd = NULL;
	m_hBitmap = NULL;
	m_iFps = 0;
	m_4dBuffersize = 256;
	m_v4dBuffer = new PW_Vector4D[m_4dBuffersize];
	m_vNormalsBuffer = new PW_Vector3D[m_4dBuffersize];
	m_ds = wireframe;
	m_bUseMaterial = false;
	m_bUseLight = false;
}

PW_3DDevice::~PW_3DDevice()
{
	this->Release();
}

bool PW_3DDevice::Create(HWND hWnd, int iWidth, int iHeight, HWND hEdit)
{
	m_hWnd = hWnd;
	m_hEdit = hEdit;
	m_iHeight = iHeight;
	m_iWidth = iWidth;
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
	return true;
}

void PW_3DDevice::Clear(PW_COLOR pwcolor, PW_FLOAT pwzbuffer)
{
	DWORD* dtmp = reinterpret_cast<DWORD*>(&pwzbuffer);
	DWORD tmp = *dtmp;
	QuadMemSet(m_pZBuffer, m_iWidth * m_iHeight * sizeof(DWORD), tmp);
	QuadMemSet(m_pBitBuffer, m_iWidth * m_iHeight * sizeof(DWORD), pwcolor);
	m_fMaxZ = 0;
	m_fMinZ = 1;
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
	char buffer[100];
	sprintf(buffer, "fps : %d, rotate angle :%.4f , maxz: %f, minz: %f", m_iFps, m_fRotate / 2.f / PI, m_fMaxZ, m_fMinZ);
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
	}
}

//void PW_3DDevice::DrawPoint(PW_POINT point, PW_COLOR pwcolor)
//{
//	if (!m_pBitBuffer)
//	{
//		return;
//	}
//	m_pBitBuffer[point.y * m_iWidth + point.x] = pwcolor;
//}

//void PW_3DDevice::DrawLine(PW_POINTF point1, PW_POINTF point2, PW_COLOR color, int isolid)
//{
//	if (!m_pBitBuffer)
//	{
//		return;
//	}
//	point1.y = m_iHeight - point1.y;
//	point2.y = m_iHeight - point2.y;
//	int dx = ROUND(point2.x) - ROUND(point1.x);
//	int dy = ROUND(point2.y) - ROUND(point1.y);
//	PW_FLOAT dz = point2.z - point1.z;
//	int steps;
//	if (abs(dx) > abs(dy))
//	{
//		steps = abs(dx);
//	}
//	else
//		steps = abs(dy);
//	if (!(ROUND(point1.y) < 0 || ROUND(point1.y) >= m_iHeight || ROUND(point1.x) < 0 || ROUND(point1.x) >= m_iWidth))
//	{
//		if (m_pZBuffer[ROUND(point1.y) * m_iWidth + ROUND(point1.x)] - point1.z> 0)
//		{
//			m_pBitBuffer[ROUND(point1.y) * m_iWidth + ROUND(point1.x)] = color;
//			m_pZBuffer[ROUND(point1.y) * m_iWidth + ROUND(point1.x)] = point1.z;
//		}
//		
//	}
//	
//	if (steps == 0)
//	{
//		return;
//	}
//	PW_FLOAT fIncrementx = dx / (PW_FLOAT)steps;
//	PW_FLOAT fIncrementy = dy / (PW_FLOAT)steps;
//	PW_FLOAT fIncrementz = dz / steps;
//	
//	PW_FLOAT fx = point1.x;
//	PW_FLOAT fy = point1.y;
//	PW_FLOAT fz = point1.z;
//	for (int i = 0; i < steps; i++)
//	{
//		if (isolid == 0)
//		{
//			i++;
//			fx += fIncrementx;
//			fy += fIncrementy;
//			fz += fIncrementz;
//		}
//		fx += fIncrementx;
//		fy += fIncrementy;
//		fz += fIncrementz;
//		if (!(ROUND(fy) < 0 || ROUND(fy) >= m_iHeight || ROUND(fx) < 0 || ROUND(fx) >= m_iWidth))
//		{
//			if (m_pZBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] - fz > 0)
//			{
//				m_pBitBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] = color;
//				m_pZBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] = fz;
//			}
//			
//		}
//		
//	}
//}

void PW_3DDevice::Update()
{
	// TODO: 在此添加任意绘图代码...
	Present();
	Clear(PW_RGBA(255, 255, 255), 1.f);
}

void PW_3DDevice::DrawLineTexture(PW_POINT3D point1, PW_POINT3D point2, int isolid /* = 1 */)
{
	if (!m_pBitBuffer)
	{
		return;
	}
	point1.y = m_iHeight - point1.y;
	point2.y = m_iHeight - point2.y;
	int dx = ROUND(point2.x) - ROUND(point1.x);
	int dy = ROUND(point2.y) - ROUND(point1.y);
	PW_FLOAT dz = point2.z - point1.z;
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
			PW_COLOR pwColor = point1.fP * m_texture->GetColor(point1.u, point1.v);
			
			m_pBitBuffer[ROUND(point1.y) * m_iWidth + ROUND(point1.x)] = pwColor;
			m_pZBuffer[ROUND(point1.y) * m_iWidth + ROUND(point1.x)] = point1.z;
			m_fMaxZ = max(point1.z, m_fMaxZ);
			m_fMinZ = min(point1.z, m_fMinZ);
		}	
	}
	if (steps == 0)
	{
		return;
	}
	PW_FLOAT fIncrementx = dx / (PW_FLOAT)steps;
	PW_FLOAT fIncrementy = dy / (PW_FLOAT)steps;
	PW_FLOAT fIncrementz = dz / steps;
	//PW_FLOAT fIncrementr = (PW_RGBA_R(point2.pwColor) - PW_RGBA_R(point1.pwColor)) / PW_FLOAT(steps);
	//PW_FLOAT fIncrementg = (PW_RGBA_G(point2.pwColor) - PW_RGBA_G(point1.pwColor)) / PW_FLOAT(steps);
	//PW_FLOAT fIncrementb = (PW_RGBA_B(point2.pwColor) - PW_RGBA_B(point1.pwColor)) / PW_FLOAT(steps);
	PW_COLORF fIncrementlp1 = (point2.fP - point1.fP) / PW_FLOAT(steps);

	PW_FLOAT fIncrementu1 = (point2.u - point1.u) / PW_FLOAT(steps);
	PW_FLOAT fIncrementv1 = (point2.v - point1.v) / PW_FLOAT(steps);


	PW_FLOAT fx = point1.x;
	PW_FLOAT fy = point1.y;
	PW_FLOAT fz = point1.z;
	//PW_FLOAT fr = PW_RGBA_R(point1.pwColor);
	//PW_FLOAT fg = PW_RGBA_G(point1.pwColor);
	//PW_FLOAT fb = PW_RGBA_B(point1.pwColor);
	PW_COLORF fLp1 = point1.fP;

	PW_FLOAT fU1 = point1.u;
	PW_FLOAT fV1 = point1.v;

	for (int i = 0; i < steps;++i)
	{
		if (isolid == 0)
		{
			i++;
			i++;
			fx += fIncrementx;
			fy += fIncrementy;
			fz += fIncrementz;
			//fr += fIncrementr;
			//fg += fIncrementg;
			//fb += fIncrementb;
			fx += fIncrementx;
			fy += fIncrementy;
			fz += fIncrementz;
			//fr += fIncrementr;
			//fg += fIncrementg;
			//fb += fIncrementb;
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
		//fr += fIncrementr;
		//fg += fIncrementg;
		//fb += fIncrementb;
		if (ROUND(fy) < 0 || ROUND(fy) >= m_iHeight || ROUND(fx) <0 || ROUND(fx) >= m_iWidth)
		{
			continue;
		}
		if (m_pZBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] - fz> 0)
		{
			PW_COLOR pwColor = fLp1 * m_texture->GetColor(fU1, fV1);
		
			m_fMaxZ = max(fz, m_fMaxZ);
			m_fMinZ = min(fz, m_fMinZ);
			m_pBitBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] = pwColor;
			m_pZBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] = fz;
		}

	}
}

void PW_3DDevice::DrawLine(PW_POINT3D point1, PW_POINT3D point2, int isolid)
{
	if (!m_pBitBuffer)
	{
		return;
	}
	point1.y = m_iHeight - point1.y;
	point2.y = m_iHeight - point2.y;
	int dx = ROUND(point2.x) - ROUND(point1.x);
	int dy = ROUND(point2.y) - ROUND(point1.y);
	PW_FLOAT dz = point2.z - point1.z;
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
			m_fMaxZ = max(point1.z, m_fMaxZ);
			m_fMinZ = min(point1.z, m_fMinZ);
		}	
	}
	if (steps == 0)
	{
		return;
	}
	PW_FLOAT fIncrementx = dx / (PW_FLOAT)steps;
	PW_FLOAT fIncrementy = dy / (PW_FLOAT)steps;
	PW_FLOAT fIncrementz = dz / steps;
	PW_FLOAT fIncrementr = (PW_RGBA_R(point2.pwColor) - PW_RGBA_R(point1.pwColor)) / PW_FLOAT(steps);
	PW_FLOAT fIncrementg = (PW_RGBA_G(point2.pwColor) - PW_RGBA_G(point1.pwColor)) / PW_FLOAT(steps);
	PW_FLOAT fIncrementb = (PW_RGBA_B(point2.pwColor) - PW_RGBA_B(point1.pwColor)) / PW_FLOAT(steps);
	//PW_COLORF fIncrementlp1 = (point2.fP - point1.fP) / PW_FLOAT(steps);

	//PW_FLOAT fIncrementu1 = (point2.u - point1.u) / PW_FLOAT(steps);
	//PW_FLOAT fIncrementv1 = (point2.v - point1.v) / PW_FLOAT(steps);
	
	
	PW_FLOAT fx = point1.x;
	PW_FLOAT fy = point1.y;
	PW_FLOAT fz = point1.z;
	PW_FLOAT fr = PW_RGBA_R(point1.pwColor);
	PW_FLOAT fg = PW_RGBA_G(point1.pwColor);
	PW_FLOAT fb = PW_RGBA_B(point1.pwColor);
	//PW_COLORF fLp1 = point1.fP;

	//PW_FLOAT fU1 = point1.u;
	//PW_FLOAT fV1 = point1.v;

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
			//fLp1 = fLp1 + fIncrementlp1;
			//
			//fU1 += fIncrementu1;
			//fV1 += fIncrementv1;
			
		}
		//fLp1 = fLp1 + fIncrementlp1;
		//
		//fU1 += fIncrementu1;
		//fV1 += fIncrementv1;
		//
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
		if (m_pZBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] - fz> 0)
		{
			PW_COLOR pwColor =  PW_RGBA(ROUND(fr), ROUND(fg), ROUND(fb));

			m_fMaxZ = max(fz, m_fMaxZ);
			m_fMinZ = min(fz, m_fMinZ);
			m_pBitBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] = pwColor;
			m_pZBuffer[ROUND(fy) * m_iWidth + ROUND(fx)] = fz;
		}
	
	}
}

void PW_3DDevice::DrawMesh(PW_Mesh& mesh)
{
	PW_Matrix4D tran;
	PW_Matrix4D tran1;
	PW_MatrixProduct4D(m_Camera->GetViewMat(), m_worldMatrix, tran);
	//PW_MatrixProduct4D(m_projMatrix, tran, tran1);
	for (int i = 0; i < mesh.pointcount;++i)
	{

		m_v4dBuffer[i] = mesh.buffer[i].MatrixProduct(tran);
		m_vNormalsBuffer[i] = mesh.buffer[i].vNormal.MatrixProduct(tran);
		PW_Vector3D pwOri;
		pwOri = pwOri.MatrixProduct(tran);
		m_vNormalsBuffer[i] = m_vNormalsBuffer[i] - pwOri;
		m_vNormalsBuffer[i].Normalize();
		//char szdebug[50];
		//sprintf(szdebug, "x:%.2f,y:%.2f,z:%.2f,w:%.2f\n", mesh.buffer[i].x, mesh.buffer[i].y, mesh.buffer[i].z,1.0f);
		//OutputDebugString(szdebug);
		//sprintf(szdebug, "x:%.2f,y:%.2f,z:%.2f,w:%.2f\n", m_v4dBuffer[i].x, m_v4dBuffer[i].y, m_v4dBuffer[i].z, m_v4dBuffer[i].w);
		//OutputDebugString(szdebug);
		//m_v4dBuffer[i].MatrixProduct(m_projMatrix);
		//m_v4dBuffer[i].NoneHomogeneous();
		//sprintf(szdebug, "x:%.2f,y:%.2f,z:%.2f,w:%.2f\n", m_v4dBuffer[i].x, m_v4dBuffer[i].y, m_v4dBuffer[i].z, m_v4dBuffer[i].w);
		//OutputDebugString(szdebug);
		//m_v4dBuffer[i].MatrixProduct(m_viewportMatrix);
		//sprintf(szdebug, "x:%.2f,y:%.2f,z:%.2f,w:%.2f\n", m_v4dBuffer[i].x, m_v4dBuffer[i].y, m_v4dBuffer[i].z, m_v4dBuffer[i].w);
		//OutputDebugString(szdebug);
	}
	for (int i = 0; i < mesh.indexcount;i+=3)
	{
		int index1 = mesh.indexbuffer[i];
		int index2 = mesh.indexbuffer[i + 1];
		int index3 = mesh.indexbuffer[i + 2];
		DrawTriPrimitive(PW_POINT3D(m_v4dBuffer[index1], mesh.buffer[index1].pwColor, m_vNormalsBuffer[index1])
			, PW_POINT3D(m_v4dBuffer[index2], mesh.buffer[index2].pwColor, m_vNormalsBuffer[index2])
			, PW_POINT3D(m_v4dBuffer[index3], mesh.buffer[index3].pwColor, m_vNormalsBuffer[index3]), PW_RGBA(255, 0, 0), m_ds);
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
			PW_FLOAT par = 1.0f;
			PW_Vector3D lightdir;
			if (m_vLights[i].iLightType == pw_lt_spotlight)
			{
			}
			else if (m_vLights[i].iLightType == pw_lt_pointlight)
			{
				PW_Vector3D vP = m_vLights[i].vPosition.MatrixProduct(viewMat);
				lightdir = vP - point;
			}
			else
			{
				PW_Vector3D vOri;
				vOri = vOri.MatrixProduct(viewMat);
				PW_Vector3D vP = m_vLights[i].vDirection.MatrixProduct(viewMat);
				lightdir = vOri - vP;
			}
			lightdir.Normalize();
			PW_FLOAT fRes = PW_DotProduct(lightdir, point.vNormal);
			if (fRes > 0)
			{
				cD = cD + m_vLights[i].cDiffuse * fRes;
			}
			//相机总是在0,0,0
			PW_Vector3D vP = point * -1;
			vP.Normalize();
			vP = vP + lightdir;
			PW_FLOAT fTmp = PW_DotProduct(point.vNormal, vP);
			fTmp = pow(fTmp, m_material.fP);
			if (fTmp > 0)
			{
				cS = cS + m_vLights[i].cSpecular * fTmp;
			}

			cP = cP + m_vLights[i].cAmbient;
		}
		
	}
	else
	{
		cP = 0;
		cD = 1;
		cS = 0;
	}
	
	//环境光
	cP = cP + m_Ambient;
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
	PW_Vector4D p1, p2, p3;
	PW_Matrix4D tan;
	p1 = point1.MatrixProduct(m_projMatrix);
	p2 = point2.MatrixProduct(m_projMatrix);
	p3 = point3.MatrixProduct(m_projMatrix);
	p1.NoneHomogeneous();
	p2.NoneHomogeneous();
	p3.NoneHomogeneous();
	p1.MatrixProduct(m_viewportMatrix);
	p2.MatrixProduct(m_viewportMatrix);
	p3.MatrixProduct(m_viewportMatrix);
	PW_FLOAT dotRes = PW_DotProduct(h, dir);

		ComputeLight(point1, m_Camera->GetViewMat());
		ComputeLight(point2, m_Camera->GetViewMat());
		ComputeLight(point3, m_Camera->GetViewMat());
	
	if (ds == wireframe)
	{
		if (dotRes > 0)
		{
			DrawLine(PW_POINT3D(p1, point1.pwColor, point1.vNormal), PW_POINT3D(p2, point2.pwColor, point2.vNormal));
			DrawLine(PW_POINT3D(p2, point2.pwColor, point2.vNormal), PW_POINT3D(p3, point3.pwColor, point3.vNormal));
			DrawLine(PW_POINT3D(p3, point3.pwColor, point3.vNormal), PW_POINT3D(p1, point1.pwColor, point1.vNormal));
		}
	}
	else
	{
		if (dotRes > 0)
		{
			DrawTriangle(PW_POINT3D(p1, point1.pwColor), PW_POINT3D(p2, point2.pwColor), PW_POINT3D(p3, point3.pwColor));
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
	
	int dy1 = ROUND(pps[1].y) - ROUND(pps[0].y);
	int dy2 = ROUND(pps[2].y) - ROUND(pps[0].y);
	int cury = pps[0].y;
	PW_POINT3D leftPoint, rightPoint;
	if (dy1 > 0)
	{
		PW_FLOAT fIncrementx1 =(ROUND(pps[1].x) - ROUND(pps[0].x)) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementx2 = (ROUND(pps[2].x) - ROUND(pps[0].x)) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementz1 = (pps[1].z - pps[0].z) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementz2 = (pps[2].z - pps[0].z) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementr1 = (PW_RGBA_R(pps[1].pwColor) - PW_RGBA_R(pps[0].pwColor)) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementg1 = (PW_RGBA_G(pps[1].pwColor) - PW_RGBA_G(pps[0].pwColor)) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementb1 = (PW_RGBA_B(pps[1].pwColor) - PW_RGBA_B(pps[0].pwColor)) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementr2 = (PW_RGBA_R(pps[2].pwColor) - PW_RGBA_R(pps[0].pwColor)) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementg2 = (PW_RGBA_G(pps[2].pwColor) - PW_RGBA_G(pps[0].pwColor)) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementb2 = (PW_RGBA_B(pps[2].pwColor) - PW_RGBA_B(pps[0].pwColor)) / PW_FLOAT(dy2);
		PW_COLORF fIncrementlp1 = (pps[1].fP - pps[0].fP) / PW_FLOAT(dy1);
		PW_COLORF fIncrementlp2 = (pps[2].fP - pps[0].fP) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementu1 = (pps[1].u - pps[0].u) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementv1 = (pps[1].v - pps[0].v) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementu2 = (pps[2].u - pps[0].u) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementv2 = (pps[2].v - pps[0].v) / PW_FLOAT(dy2);
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
			PW_SWAP(fIncrementu1, fIncrementu2, fTmp);
			PW_SWAP(fIncrementv1, fIncrementv2, fTmp);

		}
		PW_FLOAT fXl = pps[0].x;
		PW_FLOAT fXr = pps[0].x;
		PW_FLOAT fZl = pps[0].z;
		PW_FLOAT fZr = pps[0].z;
		
		PW_FLOAT fR1 = PW_RGBA_R(pps[0].pwColor);
		PW_FLOAT fG1 = PW_RGBA_G(pps[0].pwColor);
		PW_FLOAT fB1 = PW_RGBA_B(pps[0].pwColor);
		PW_FLOAT fR2 = PW_RGBA_R(pps[0].pwColor);
		PW_FLOAT fG2 = PW_RGBA_G(pps[0].pwColor);
		PW_FLOAT fB2 = PW_RGBA_B(pps[0].pwColor);
		PW_COLORF fLp1 = pps[0].fP;
		PW_COLORF fLp2 = pps[0].fP;
		PW_FLOAT fU1 = pps[0].u;
		PW_FLOAT fV1 = pps[0].v;
		PW_FLOAT fU2 = pps[0].u;
		PW_FLOAT fV2 = pps[0].v;
		int curY = ROUND(pps[0].y);

		/*if (!(pps[0].y < 0 || pps[0].y >= m_iHeight || pps[0].x < 0 || pps[0].x >= m_iWidth))
		{
			m_pBitBuffer[ROUND(pps[0].y) * m_iWidth + ROUND(pps[0].x)] = pps[0].pwColor;
		}*/
		if (m_bUseTexture)
		{
			DrawLineTexture(pps[0], pps[0]);
		}
		else
			DrawLine(pps[0], pps[0]);
		DrawLine(pps[0], pps[0]);
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


			fLp1 =fLp1 + fIncrementlp1;
			fLp2 =fLp1 + fIncrementlp2;
			fU1 += fIncrementu1;
			fV1 += fIncrementv1;
			fU2 += fIncrementu2;
			fV2 += fIncrementv2;
			if (k == dy1 - 1)
			{
				leftPoint.x = ROUND(fXl);
				leftPoint.y = curY;
				leftPoint.z = fZl;
				leftPoint.fP = fLp1;
				leftPoint.u = fU1;
				leftPoint.v = fV1;
				leftPoint.pwColor = PW_RGBA(ROUND(fR1), ROUND(fG1), ROUND(fB1));
				rightPoint.x = ROUND(fXr);
				rightPoint.y = curY;
				rightPoint.pwColor = PW_RGBA(ROUND(fR2), ROUND(fG2), ROUND(fB2));
				rightPoint.z = fZr;
				rightPoint.fP = fLp2;
				rightPoint.u = fU2;
				rightPoint.v = fV2;
			}
			PW_POINT3D p1, p2;
			p1.x = fXl;
			p1.y = curY;
			p1.z = fZl;
			p1.fP = fLp1;
			p1.u = fU1;
			p1.v = fV1;
			p1.pwColor = PW_RGBA(ROUND(fR1), ROUND(fG1), ROUND(fB1));
			p2.x = fXr;
			p2.y = curY;
			p2.z = fZr;
			p2.pwColor = PW_RGBA(ROUND(fR2), ROUND(fG2), ROUND(fB2));
			p2.fP = fLp2;
			p2.u = fU1;
			p2.v = fV1;
			if (m_bUseTexture)
			{
				DrawLineTexture(p1, p2);
			}
			else
				DrawLine(p1, p2);
		}
		
	}
	else
	{
		if (pps[0].x < pps[1].x)
		{
			leftPoint = pps[0];
			rightPoint = pps[1];
		}
		else
		{
			leftPoint = pps[1];
			rightPoint = pps[0];
		}
		
		
	}
	if (dy2 != 0 && dy2 > dy1)
	{
		dy1 = ROUND(pps[2].y) - ROUND(leftPoint.y);
		dy2 = ROUND(pps[2].y) - ROUND(rightPoint.y);
		PW_FLOAT fIncrementx1 = (ROUND(pps[2].x) - ROUND(leftPoint.x)) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementx2 = (ROUND(pps[2].x) - ROUND(rightPoint.x)) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementz1 = (pps[2].z - leftPoint.z) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementz2 = (pps[2].z - rightPoint.z) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementr1 = (PW_RGBA_R(pps[2].pwColor) - PW_RGBA_R(leftPoint.pwColor)) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementg1 = (PW_RGBA_G(pps[2].pwColor) - PW_RGBA_G(leftPoint.pwColor)) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementb1 = (PW_RGBA_B(pps[2].pwColor) - PW_RGBA_B(leftPoint.pwColor)) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementr2 = (PW_RGBA_R(pps[2].pwColor) - PW_RGBA_R(rightPoint.pwColor)) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementg2 = (PW_RGBA_G(pps[2].pwColor) - PW_RGBA_G(rightPoint.pwColor)) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementb2 = (PW_RGBA_B(pps[2].pwColor) - PW_RGBA_B(rightPoint.pwColor)) / PW_FLOAT(dy2);
		PW_COLORF fIncrementlp1 = (pps[2].fP - leftPoint.fP) / PW_FLOAT(dy1);
		PW_COLORF fIncrementlp2 = (pps[2].fP - rightPoint.fP) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementu1 = (pps[2].u - leftPoint.u) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementv1 = (pps[2].v - leftPoint.v) / PW_FLOAT(dy1);
		PW_FLOAT fIncrementu2 = (pps[2].u - rightPoint.u) / PW_FLOAT(dy2);
		PW_FLOAT fIncrementv2 = (pps[2].v - rightPoint.v) / PW_FLOAT(dy2);
		int steps = dy1;
		PW_FLOAT fCurR1 = PW_RGBA_R(leftPoint.pwColor);
		PW_FLOAT fCurG1 = PW_RGBA_G(leftPoint.pwColor);
		PW_FLOAT fCurB1 = PW_RGBA_B(leftPoint.pwColor);
		PW_FLOAT fCurR2 = PW_RGBA_R(rightPoint.pwColor);
		PW_FLOAT fCurG2 = PW_RGBA_G(rightPoint.pwColor);
		PW_FLOAT fCurB2 = PW_RGBA_B(rightPoint.pwColor);
		PW_FLOAT fx1 = leftPoint.x;
		PW_FLOAT fx2 = rightPoint.x;
		PW_FLOAT fz1 = leftPoint.z;
		PW_FLOAT fz2 = rightPoint.z;
		PW_COLORF flp1 = leftPoint.fP;
		PW_COLORF flp2 = rightPoint.fP;
		PW_FLOAT fU1 = leftPoint.u;
		PW_FLOAT fV1 = leftPoint.v;
		PW_FLOAT fU2 = rightPoint.u;
		PW_FLOAT fV2 = rightPoint.v;
		int curY = leftPoint.y;
		for (int k = 0; k < steps;++k)
		{
			fx1 += fIncrementx1;
			fx2 += fIncrementx2;
			fz1 += fIncrementz1;
			fz2 += fIncrementz2;
			flp1 = flp1 + fIncrementlp1;
			flp2 = flp1 + fIncrementlp2;
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
			p1.z = fz1;
			p1.fP = flp1;
			p1.u = fU1;
			p1.v = fV1;

			p1.pwColor = PW_RGBA(ROUND(fCurR1), ROUND(fCurG1), ROUND(fCurB1));
			p2.x = fx2;
			p2.y = curY;
			p2.z = fz1;
			p2.fP = flp2;
			p2.u = fU2;
			p2.v = fV2;
			p2.pwColor = PW_RGBA(ROUND(fCurR2), ROUND(fCurG2), ROUND(fCurB2));
			if (m_bUseTexture)
			{
				DrawLineTexture(p1, p2);
			}
			else
				DrawLine(p1, p2);
		}
	}
	//if (m_bUseTexture)
	//{
	//	DrawLineTexture(point1, point2);
	//	DrawLineTexture(point2, point3);
	//	DrawLineTexture(point3, point1);
	//}
	//else
	//{
	//	DrawLine(point1, point2);
	//	DrawLine(point2, point3);
	//	DrawLine(point3, point1);
	//}
	
}
