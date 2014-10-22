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
		u = 0;
		v = 0;
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
	inline PW_COLOR BiLinerGetColor(PW_FLOAT u, PW_FLOAT v)
	{
		//v = 1 - v;
		PW_CLAMP(v, 0.f, 1.f);
		PW_CLAMP(u, 0.f, 1.f);
		PW_FLOAT x = u * (bih.biWidth - 1);
		PW_FLOAT y = v * (bih.biHeight - 1);
		PW_FLOAT x0 = PW_INT(x);
		PW_FLOAT y0 = PW_INT(y);
		PW_FLOAT x1 = x - x0;
		PW_FLOAT y1 = y - y0;
		PW_FLOAT x2 = x0 + 1 - x;
		PW_FLOAT y2 = y0 + 1 - y;
		PW_FLOAT r1 = x2 * y2;
		PW_FLOAT r2 = x1 * y2;
		PW_FLOAT r3 = x2 * y1;
		PW_FLOAT r4 = x1 * y1;
		PW_INT r, g, b, r11, g11, b11;
		r = 0;
		g = 0;
		b = 0;
		GetColorI(x0, y0, r1, r11,g11,b11);
		r += r11;
		g += g11;
		b += b11;
		GetColorI(x0 + 1, y0, r2, r11, g11, b11);
		r += r11;
		g += g11;
		b += b11;
		GetColorI(x0, y0 + 1, r3, r11, g11, b11);
		r += r11;
		g += g11;
		b += b11;
		GetColorI(x0 + 1, y0 + 1, r4, r11, g11, b11);
		r += r11;
		g += g11;
		b += b11;
		PW_CLAMP(r, 0, 255);
		PW_CLAMP(g, 0, 255);
		PW_CLAMP(b, 0, 255);
		PW_COLOR ret = PW_RGBA(r,g,b);
		return ret;
	}

	inline void GetColorI(PW_INT x, PW_INT y, PW_FLOAT f, PW_INT& r,  PW_INT& g, PW_INT& b)
	{
		if (x >= bih.biWidth || y >= bih.biHeight || x < 0 || y < 0)
		{
			r = 0;
			g = 0;
			b = 0;
			return ;
		}
		y = bih.biHeight - 1 - y;
		if (bih.biBitCount == 24)
		{
			b = ROUND(pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3] * f);
			g = ROUND(pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3 + 1] * f);
			r = ROUND(pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3 + 2] * f);
			
		}
		else
		{
			 b = ROUND(pBuffer[y * bih.biWidth * 4 + x * 4] * f);
			 g = ROUND(pBuffer[y * bih.biWidth * 4 + x * 4 + 1] * f);
			 r = ROUND(pBuffer[y * bih.biWidth * 4 + x * 4 + 2] * f);
			
		}
	}

	inline PW_COLOR GetColorI(PW_INT x, PW_INT y)
	{
		PW_COLOR ret = 0;
		if (x >= bih.biWidth || y >= bih.biHeight || x < 0 || y < 0)
		{
			return ret;
		}
		if (bih.biBitCount == 24)
		{
			PW_INT b = pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3];
			PW_INT g = pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3 + 1];
			PW_INT r = pBuffer[y * (bih.biWidth * 3 + iSpan) + x * 3 + 2];
			ret = PW_RGB(r, g, b);
		}
		else
		{
			PW_INT b = pBuffer[y * bih.biWidth * 4 + x * 4];
			PW_INT g = pBuffer[y * bih.biWidth * 4 + x * 4 + 1];
			PW_INT r = pBuffer[y * bih.biWidth * 4 + x * 4 + 2];
			ret = PW_RGB(r, g, b);
		}
		return ret;
	}
	
	//u v转x y
	inline void GetXY(PW_FLOAT u, PW_FLOAT v, PW_FLOAT& x, PW_FLOAT& y)
	{
		v = 1 - v;
		x = (u * (bih.biWidth - 1));
		y = (v * (bih.biHeight - 1));
	}

	inline PW_COLOR GetColor(PW_FLOAT u, PW_FLOAT v)
	{
		v = 1 - v;
		PW_INT x = ROUND(u * (bih.biWidth - 1));
		PW_INT y = ROUND(v * (bih.biHeight - 1));
		return GetColorI(x, y);
	}

	//单点
	//inline PW_COLOR GetColor(PW_FLOAT fx, PW_FLOAT fy)
	//{
	//	PW_INT x = ROUND(fx);
	//	PW_INT y = ROUND(fy);
	//	return GetColorI(x, y);
	//}

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