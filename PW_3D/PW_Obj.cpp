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
	for (int i = 0; i < indexcount;i+=3)
	{
		int index1 = indexbuffer[i];
		int index2 = indexbuffer[i + 1];
		int index3 = indexbuffer[i + 2];
		PW_Vector3D v = buffer[index2] - buffer[index1];
		PW_Vector3D q = buffer[index3] - buffer[index2];
		PW_Vector3D nor;
		PW_CrossProduct(v, q, nor);
		nor.Normalize();
		vNormals[index1].push_back(nor);
		vNormals[index2].push_back(nor);
		vNormals[index3].push_back(nor);
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

bool PW_Texture::LoadBitmap(LPCSTR lpszFile)
{
	if(lpszFile == NULL)
		return NULL;
	HBITMAP hBitmap;
	HANDLE hf;
	BITMAPFILEHEADER* pbmfh;
	DWORD dwBytesRead, dwFileSize, dwFileSizeHigh;
	BOOL bSuccess;
	// ��һ��bmp�ļ�
	hf = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if( hf == INVALID_HANDLE_VALUE)
	{
		TRACE("Open file filed with error %d ", GetLastError());
		return NULL;
	}
	// �õ�����ļ���С
	dwFileSize = GetFileSize(hf, &dwFileSizeHigh);
	if( dwFileSizeHigh )
	{
		CloseHandle(hf);
		return NULL;
	}
	// �����ڴ�,��СΪ���ļ��Ĵ�С
	pbmfh = (BITMAPFILEHEADER*)malloc(dwFileSize);
	if( !pbmfh )
	{
		CloseHandle(hf);
		return NULL;
	}
	// ��ȡ����
	bSuccess = ReadFile(hf, pbmfh, dwFileSize, &dwBytesRead, NULL);
	CloseHandle(hf);
	// Ч���ļ���С���ļ���ʽ
	if( !bSuccess || dwFileSize != dwBytesRead
		|| pbmfh->bfType != 0x4D42 || pbmfh->bfSize != dwFileSize)
	{
		free((void*)pbmfh);
		return NULL;
	}
	// ����DIBת��
	hBitmap = CreateDIBitmap(
		GetWindowDC(NULL),
		(BITMAPINFOHEADER*)(pbmfh + 1),
		CBM_INIT,
		(BYTE*)pbmfh + pbmfh->bfOffBits,
		(BITMAPINFO*)(pbmfh + 1),
		DIB_RGB_COLORS);
	free((void*)pbmfh);
	return hBitmap;
}