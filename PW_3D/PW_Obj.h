//#ifndef PW_OBJ
//#define PW_OBJ
#pragma once

#include "PW_Math.h"


typedef PW_Vector3D PW_POINTF;
//typedef PW_Vector3D PW_POINT3D;

struct PW_Vertex:public PW_Vector3D
{
	PW_COLOR pwColor;
	PW_Vector3D vNormal;
	PW_COLORF fP;
	PW_FLOAT u;
	PW_FLOAT v;
	PW_Vertex(PW_FLOAT fx = 0., PW_FLOAT fy = 0., PW_FLOAT fz = 0., PW_COLOR color = PW_RGBA(255, 255, 255))
		: pwColor(color)
	{
		this->x = fx;
		this->y = fy;
		this->z = fz;
	}

	PW_Vertex(PW_Vector4D& v4d, PW_COLOR color, PW_Vector3D nor = PW_Vector3D(0,0,0), PW_FLOAT u = 0, PW_FLOAT v = 0, PW_COLORF fP = PW_COLORF())
	{
		this->x = v4d.x;
		this->y = v4d.y;
		this->z = v4d.z;
		this->vNormal = nor;
		this->pwColor = color;
		this->u = u;
		this->v = v;
		this->fP = fP;
	}
};

typedef PW_Vertex PW_POINT3D;

struct PW_POINT
{
	int x;
	int y;
	PW_COLOR pwColor;
	PW_POINT(int xx = 0, int yy = 0) :x(xx), y(yy)
	{}
};

struct PW_Material
{
	PW_COLORF cAmbient;
	PW_COLORF cDiffuse;
	PW_COLORF cEmission;
	PW_COLORF cSpecularReflection;
	PW_FLOAT fP;//镜面反射强度
};

struct PW_Triangle
{
	int index1;
	int index2;
	int index3;
	PW_FLOAT u1;
	PW_FLOAT u2;
	PW_FLOAT u3;
	PW_FLOAT v1;
	PW_FLOAT v2;
	PW_FLOAT v3;
	int& operator[](int index)
	{
		switch(index)
		{
		case 0:
			return index1;
			break;
		case 1:
			return index2;
			break;
		case 2:
			return index3;
			break;
		default:
			return index1;
		}
	}
};

struct PW_Mesh
{
	PW_Vertex* buffer;
	PW_Triangle* indexbuffer;
	int pointcount;
	//int* indexbuffer;
	int indexcount;
	PW_Mesh():buffer(NULL), indexbuffer(NULL), pointcount(0), indexcount(0)
	{}


	void SetBuffer(PW_Vertex* pBuffer, PW_Triangle* pTriBuffer, int points, int indeces)
	{
		buffer = pBuffer;
		//indexbuffer = pIndexBuffer;
		pointcount = points;
		indexcount = indeces;
		indexbuffer = pTriBuffer;
		ComputeNormal();
	}

	int GetVertexCount(){ return pointcount; }

	void ComputeNormal();

	void Release()
	{
		if (buffer)
		{
			delete[] buffer;
			buffer = NULL;
		}
		if (indexbuffer)
		{
			delete[] indexbuffer;
			indexbuffer = NULL;
		}
		pointcount = 0;
		indexcount = 0;
	}
};

enum PW_LIGHT_TYPE
{
	pw_lt_pointlight,
	pw_lt_directionallight,
	pw_lt_spotlight,
};

struct PW_Light
{
	PW_FLOAT fAttenuation0;
	PW_FLOAT fAttenuation1;
	PW_FLOAT fAttenuation2;
	PW_COLORF cDiffuse;
	PW_COLORF cAmbient;
	PW_COLORF cSpecular;
	PW_INT iLightType;
	PW_Vector3D vPosition;
	PW_Vector3D vDirection;
	PW_FLOAT fRange;
	PW_Light()
	{
		fAttenuation0 = 0;
		fAttenuation1 = 0;
		fAttenuation2 = 0;
		
	}
	PW_Light(PW_FLOAT p1, PW_FLOAT p2, PW_FLOAT p3, PW_COLORF cDif, PW_COLORF cAmb, PW_COLORF cSpe, PW_INT lightType,
		PW_Vector3D pos, PW_Vector3D dir, PW_FLOAT fRan)
		:fAttenuation0(p1), fAttenuation1(p2), fAttenuation2(p3),
		cDiffuse(cDif), cAmbient(cAmb), cSpecular(cSpe), iLightType(lightType),
		vPosition(pos), vDirection(dir), fRange(fRan)
	{}
};

struct PW_Texture
{
	LPCSTR strFileName;
	inline PW_COLOR GetColor(PW_FLOAT u, PW_FLOAT v)
	{
		v = 1 - v;
		PW_INT x = ROUND(u * bih.biWidth);
		PW_INT y = ROUND(v * bih.biHeight);
		PW_COLOR ret;
		if (bih.biBitCount == 24)
		{
			PW_INT b = pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3];
			PW_INT g = pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3 + 1];
			PW_INT r = pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3 + 2];
			ret = PW_RGB(r,g,b);
		}
		else
		{
			PW_INT b = pBuffer[y * bih.biWidth * 4 + x * 4];
			PW_INT g = pBuffer[y * bih.biWidth * 4 + x * 4 + 1];
			PW_INT r = pBuffer[y * bih.biWidth * 4 + x * 4 + 2];
			ret = PW_RGB(r,g,b);
		}
		return ret;
	}
	PW_BYTE * pBuffer;
	BITMAPINFOHEADER bih;
	PW_INT iSpan;
	PW_Texture()
	{
		strFileName = NULL;
		pBuffer = NULL;
	}
	~PW_Texture()
	{
		if (pBuffer)
		{
			delete[] pBuffer;
			pBuffer = NULL;
		}
	}

	bool LoadBitmap(LPCSTR strFile);
};
//#endif