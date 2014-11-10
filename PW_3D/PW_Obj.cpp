#include "PW_Obj.h"
#include <vector>

using namespace std;

void PW_Mesh::ComputeNormal()
{
	if (indexcount == 0)
	{
		return;
	}
	vector<vector<PW_Vector3D>> vNormals;
	vNormals.resize(pointcount);
	for (int i = 0; i < indexcount;i++)
	{
		int index1 = indexbuffer[i][0];
		int index2 = indexbuffer[i][1];
		int index3 = indexbuffer[i][2];
		PW_Vector3D v = buffer[index2] - buffer[index1];
		PW_Vector3D q = buffer[index3] - buffer[index2];
		PW_Vector3D nor;
		PW_CrossProduct(v, q, nor);
		nor.Normalize();
		bool isExist = false;
		for (int j = 0; j < vNormals[index1].size();j++)
		{
			if (abs(PW_DotProduct(vNormals[index1][j], nor) - 1.) < EPSILON)
			{
				isExist = true;
				break;
			}
		}
		if (!isExist)
		{
			vNormals[index1].push_back(nor);
		}
		isExist = false;
		for (int j = 0; j < vNormals[index2].size();j++)
		{
			if (abs(PW_DotProduct(vNormals[index2][j], nor) - 1.) < EPSILON)
			{
				isExist = true;
				break;
			}
		}
		if (!isExist)
		{
			vNormals[index2].push_back(nor);
		}
		isExist = false;
		for (int j = 0; j < vNormals[index3].size();j++)
		{
			if (abs(PW_DotProduct(vNormals[index3][j], nor) - 1.) < EPSILON)
			{
				isExist = true;
				break;
			}
		}
		if (!isExist)
		{
			vNormals[index3].push_back(nor);
		}
		
	}
	for (int i = 0; i < pointcount;i++)
	{
		PW_Vector3D nor;
		for (int k = 0; k < vNormals[i].size(); ++k)
		{
			nor = nor + vNormals[i][k];
			//nor.Normalize();
		}
		nor.Normalize();
		buffer[i].vNormal = nor;
	}
}

//inline PW_COLOR PW_Texture::GetColor(PW_FLOAT u, PW_FLOAT v)
//{
//	v = 1 - v;
//	PW_INT x = ROUND(u * bih.biWidth);
//	PW_INT y = ROUND(v * bih.biHeight);
//	PW_COLOR ret;
//	if (bih.biBitCount == 24)
//	{
//		PW_INT b = pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3];
//		PW_INT g = pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3 + 1];
//		PW_INT r = pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3 + 2];
//		ret = PW_RGB(r,g,b);
//	}
//	else
//	{
//		PW_INT b = pBuffer[y * bih.biWidth * 4 + x * 4];
//		PW_INT g = pBuffer[y * bih.biWidth * 4 + x * 4 + 1];
//		PW_INT r = pBuffer[y * bih.biWidth * 4 + x * 4 + 2];
//		ret = PW_RGB(r,g,b);
//	}
//	return ret;
//}

bool PW_Texture::LoadBitmap(LPCSTR lpszFile)
{
	if(lpszFile == NULL)
		return false;

	HANDLE hf;
	BITMAPFILEHEADER* pbmfh;
	BITMAPINFO *bif;
	DWORD dwBytesRead, dwFileSize, dwFileSizeHigh;
	BOOL bSuccess;
	// 打开一个bmp文件
	hf = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if( hf == INVALID_HANDLE_VALUE)
	{
		//TRACE("Open file filed with error %d ", GetLastError());
		return false;
	}
	// 得到这个文件大小
	dwFileSize = GetFileSize(hf, &dwFileSizeHigh);
	if( dwFileSizeHigh )
	{
		CloseHandle(hf);
		return false;
	}
	// 分配内存,大小为该文件的大小
	pbmfh = (BITMAPFILEHEADER*)malloc(dwFileSize);
	if( !pbmfh )
	{
		CloseHandle(hf);
		return false;
	}
	// 读取数据
	bSuccess = ReadFile(hf, pbmfh, dwFileSize, &dwBytesRead, NULL);
	CloseHandle(hf);
	// 效验文件大小和文件格式
	if( !bSuccess || dwFileSize != dwBytesRead
		|| pbmfh->bfType != 0x4D42 || pbmfh->bfSize != dwFileSize)
	{
		free((void*)pbmfh);
		return false;
	}
	
	bif = (BITMAPINFO *)(pbmfh + 1);
	if (bif->bmiHeader.biBitCount == 24)
	{
		iSpan = (4 - bif->bmiHeader.biWidth * 3 % 4) % 4;
	}
	pBuffer = new PW_BYTE[bif->bmiHeader.biSizeImage];
	memcpy(pBuffer, bif->bmiColors, bif->bmiHeader.biSizeImage);
	bih = bif->bmiHeader;
	free((void*)pbmfh);
	return true;
}

PW_BOOL PW_Mesh::RayReflect(PW_LightRay& lightRay, PW_LightRay& reflectLight1, PW_LightRay& reflectLight2)
{
	for (int i = 0; i < indexcount;i++)
	{
		int index1 = indexbuffer[i][0];
		int index2 = indexbuffer[i][1];
		int index3 = indexbuffer[i][2];
		PW_TrianglePlane tp;
		
		tp.p1 = pNowBuffer[index1];
		tp.p2 = pNowBuffer[index2];
		tp.p3 = pNowBuffer[index3];
		PW_Vector3D pinsert;
		PW_Vector3D vRef1, vRef2;
		PW_FLOAT fRer = 0;
		PW_INT nRes = 0;
		PW_Vector3D vNorrr;
		if (RayInserctionPlane(lightRay.vStart, lightRay.vDir, tp, pinsert, vRef1, vRef2, fRer, vNorrr))
		{
			nRes++;
			reflectLight1.vStart = pinsert;
			reflectLight1.vDir = vRef1;
			reflectLight1.cAmbient = material.cAmbient;
			reflectLight1.cDiffuse = material.cDiffuse;
			reflectLight1.cEmission = material.cEmission;
			reflectLight1.cSpecularReflection = material.cSpecularReflection;
			reflectLight1.vNormal = vNorrr;
			if (fRer > 0)
			{
				reflectLight2.vDir = vRef2;
				reflectLight2.vStart = pinsert;
				reflectLight2.cAmbient = material.cAmbient;
				reflectLight2.cDiffuse = material.cDiffuse;
				reflectLight2.cEmission = material.cEmission;
				reflectLight2.cSpecularReflection = material.cSpecularReflection;
				reflectLight2.vNormal = vNorrr;
				nRes++;
			}
			return nRes;
		}	
	}
	return 0;
}

void PW_Mesh::ComputeCurVertex()
{
	PW_Matrix4D tran = absoluteTM;
	for (int i = 0; i < pointcount; ++i)
	{
		pNowBuffer[i] = buffer[i];
		PW_Vector4D vTmp = buffer[i].MatrixProduct(tran);
		//pNowBuffer[i] = buffer[i].MatrixProduct(tran);
		PW_Vector3D vNor = buffer[i].vNormal.MatrixProduct(tran);
		
		PW_Vector3D pwOri;
		pwOri = pwOri.MatrixProduct(tran);
		vNor = vNor - pwOri;
		vNor.Normalize();
		pNowBuffer[i].vNormal = vNor;
		pNowBuffer[i].x = vTmp.x;
		pNowBuffer[i].y = vTmp.y;
		pNowBuffer[i].z = vTmp.z;
 	}
}