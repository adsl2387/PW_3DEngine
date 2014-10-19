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
	PW_Vertex(PW_FLOAT fx = 0., PW_FLOAT fy = 0., PW_FLOAT fz = 0., PW_COLOR color = PW_RGBA(255, 255, 255))
		: pwColor(color)
	{
		this->x = fx;
		this->y = fy;
		this->z = fz;
	}

	PW_Vertex(PW_Vector4D& v4d, PW_COLOR color, PW_Vector3D nor = PW_Vector3D(0,0,0))
	{
		this->x = v4d.x;
		this->y = v4d.y;
		this->z = v4d.z;
		this->vNormal = nor;
		this->pwColor = color;
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

struct PW_Mesh
{
	PW_Vertex* buffer;
	int pointcount;
	int* indexbuffer;
	int indexcount;
	PW_Mesh():buffer(NULL), indexbuffer(NULL), pointcount(0), indexcount(0)
	{}

	void SetBuffer(PW_Vertex* pBuffer, int* pIndexBuffer, int points, int indeces)
	{
		buffer = pBuffer;
		indexbuffer = pIndexBuffer;
		pointcount = points;
		indexcount = indeces;
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
	PW_Light(PW_FLOAT p1, PW_FLOAT p2, PW_FLOAT p3, PW_COLOR cDif, PW_COLOR cAmb, PW_COLOR cSpe, PW_INT lightType,
		PW_Vector3D pos, PW_Vector3D dir, PW_FLOAT fRan)
		:fAttenuation0(p1), fAttenuation1(p2), fAttenuation2(p3),
		cDiffuse(cDif), cAmbient(cAmb), cSpecular(cSpe), iLightType(lightType),
		vPosition(pos), vDirection(dir), fRange(fRan)
	{}
};

struct PW_Texture
{
	LPCSTR strFileName;

};
//#endif