#pragma once
#include <windows.h>
#include <math.h>
typedef float PW_FLOAT;
typedef int PW_INT;
typedef double PW_DOUBLE;
typedef unsigned char PW_BYTE;
typedef int PW_BOOL;
#define PW_TRUE	1
#define PW_FALSE	0

#define PI 3.1415926535898f
#define EPSILON 0.0001f
#define ROUND(x) ((int)((x) + 0.5f))
typedef DWORD PW_COLOR;

#define PW_RGB(r,g,b) (((b) << 0 ) | ((g) << 8) | ((r) << 16))

#define PW_RGBA(r,g,b) (PW_RGB(r,g,b) << 0)

#define PW_CLAMP(v,a,b) {if ((v) < (a)) (v) = (a); if((v) > (b)) (v) = (b);}

#define PW_RGBA_R(x) ((PW_INT)(((x) >> 16) & 0xff)) 
#define PW_RGBA_G(x) ((PW_INT)(((x) >> 8) & 0xff)) 
#define PW_RGBA_B(x) ((PW_INT)(((x) >> 0) & 0xff)) 
#define PW_RGBA_A(x) ((PW_INT)(((x) >> 24) & 0xff)) 

#define PW_SWAP(a,b,t) t = a;a = b;b=t;

struct PW_Matrix4D
{
	PW_FLOAT mat[4][4];

	PW_Matrix4D()
	{
		Zero();
	}

	inline PW_FLOAT* operator[](int index)
	{
		return mat[index];
	}

	inline void Zero()
	{
		memset(mat, 0, sizeof(mat));
	}

	inline void IdentityMatrix()
	{
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j <4;++j)
			{
				mat[i][j] = (i == j);
			}
		}
	}
};

struct PW_Vector4D;

struct PW_Vector3D
{
	PW_FLOAT x;
	PW_FLOAT y;
	PW_FLOAT z;
	//PW_COLOR pwColor;
	PW_Vector3D(PW_FLOAT fx = 0.f, PW_FLOAT fy=0.f, PW_FLOAT fz=0.f)
		:x(fx), y(fy), z(fz)
	{}

	PW_Vector3D(PW_Vector4D& v4d);

	PW_BOOL IsEqual(PW_Vector3D& v)
	{
		if (abs(v.x - x) < EPSILON && abs(v.y - y) < EPSILON && abs(v.z - z) < EPSILON)
		{
			return PW_TRUE;
		}
		else
		{
			return PW_FALSE;
		}
	}

	PW_BOOL IsZero()
	{
		if (abs(0.f - x) < EPSILON && abs(0.f - y) < EPSILON && abs(0.f - z) < EPSILON)
		{
			return PW_TRUE;
		}
		else
		{
			return PW_FALSE;
		}
	}

	inline PW_BOOL operator>(PW_Vector3D& v3d)
	{
		return x > v3d.x && y > v3d.y && z > v3d.z;
	}

	void operator=(PW_Vector4D& v4d);

	inline PW_Vector3D operator+ (PW_Vector3D& vec)
	{
		PW_Vector3D retv;
		retv.x = x + vec.x;
		retv.y = y + vec.y;
		retv.z = z + vec.z;
		return retv;
	}

	inline PW_Vector3D operator- (PW_Vector3D& vec)
	{
		PW_Vector3D retv;
		retv.x = x - vec.x;
		retv.y = y - vec.y;
		retv.z = z - vec.z;
		return retv;
	}

	inline PW_Vector3D operator* (PW_FLOAT fvalue)
	{
		PW_Vector3D retv;
		retv.x = x * fvalue;
		retv.y = y * fvalue;
		retv.z = z * fvalue;
		return retv;
	}

	inline PW_Vector3D operator/ (PW_FLOAT fvalue)
	{
		PW_Vector3D retv;
		retv.x = x / fvalue;
		retv.y = y / fvalue;
		retv.z = z / fvalue;
		return retv;
	}

	//inline void operator= (PW_Vector4D& v4d)
	//{
	//	x = v4d.x / v4d.w;
	//	y = v4d.y / v4d.w;
	//	z = v4d.z / v4d.w;
	//}

	PW_Vector4D MatrixProduct(PW_Matrix4D& mat);

	inline PW_FLOAT GetLen()
	{
		return sqrt(x * x + y * y + z* z);
	}

	inline PW_FLOAT GetLen2()
	{
		return (x * x + y * y + z* z);
	}

	void Normalize()
	{
		PW_FLOAT flen = sqrt(x * x + y * y + z* z);
		if (flen != 0)
		{
			x /= flen;
			y /= flen;
			z /= flen;
		}
	}
};

struct PW_Vector4D
{
	PW_FLOAT x;
	PW_FLOAT y;
	PW_FLOAT z;
	PW_FLOAT w;

	PW_COLOR pwColor;
	PW_Vector4D(PW_FLOAT fx = 0.f, PW_FLOAT fy=0.f, PW_FLOAT fz=0.f, PW_FLOAT fw = 1.f)
		:x(fx),y(fy),z(fz),w(fw)
	{}

	PW_Vector4D(PW_Vector3D& vec, PW_FLOAT fW = 1.f)
		:x(vec.x),y(vec.y),z(vec.z),w(fW)
	{}

	void MatrixProduct(PW_Matrix4D& mat);

	void NoneHomogeneous()
	{
		if (w != 0.f)
		{
			x /= w;
			y /= w;
			z /= w;
			w = 1;
		}
	}
};

struct PW_COLORF
{
	PW_FLOAT r;
	PW_FLOAT g;
	PW_FLOAT b;
	PW_FLOAT a;

	PW_COLORF(PW_FLOAT rr = 0, PW_FLOAT gg = 0, PW_FLOAT bb = 0, PW_FLOAT aa = 0)
		:r(rr), g(gg), b(bb), a(aa)
	{}

	PW_COLORF operator*(PW_COLORF& c)
	{
		PW_COLORF ret;
		ret.r = r * c.r;
		ret.b = b * c.b;
		ret.g = g * c.g;
		ret.a = a * c.a;
		return ret;
	}
	PW_COLORF operator*(PW_FLOAT c)
	{
		PW_COLORF ret;
		ret.r = r * c;
		ret.b = b * c;
		ret.g = g * c;
		ret.a = a * c;
		return ret;
	}

	PW_COLORF operator+(PW_COLORF& c)
	{
		PW_COLORF ret;
		ret.r = r + c.r;
		ret.b = b + c.b;
		ret.g = g + c.g;
		ret.a = a + c.a;
		return ret;
	}

	void operator+= (PW_COLORF& c)
	{
		r += c.r;
		g += c.g;
		b += c.b;
		a += c.a;
	}

	PW_COLOR operator*(PW_COLOR pwColor)
	{
		int iR = ROUND(PW_RGBA_R(pwColor) * r);
		int iG = ROUND(PW_RGBA_G(pwColor) * g);
		int iB = ROUND(PW_RGBA_B(pwColor) * b);
		int iA = ROUND(PW_RGBA_A(pwColor) * a);
		PW_CLAMP(iR, 0, 255);
		PW_CLAMP(iG, 0, 255);
		PW_CLAMP(iB, 0, 255);
		PW_CLAMP(iA, 0, 255);
		return PW_RGBA(iR, iG, iB);
	}

	PW_COLORF operator-(PW_COLORF& c)
	{
		PW_COLORF ret;
		ret.r = r - c.r;
		ret.b = b - c.b;
		ret.g = g - c.g;
		ret.a = a - c.a;
		return ret;
	}

	PW_COLORF operator/(PW_FLOAT c)
	{
		PW_COLORF ret;
		if (c == 0)
		{
			return ret;
		}
		ret.r = r / c;
		ret.b = b / c;
		ret.g = g / c;
		ret.a = a / c;
		return ret;
	}
};

struct PW_Line
{
	PW_Vector3D oripoint;
	PW_Vector3D dirction;
};

struct PW_TrianglePlane
{
	PW_Vector3D p1;
	PW_Vector3D p2;
	PW_Vector3D p3;
	PW_Vector3D n1;
	PW_Vector3D n2;
	PW_Vector3D n3;
};

struct PW_Plane
{
	PW_Vector3D oripoint;
	PW_Vector3D normal;
};

inline void PW_CrossProduct(PW_Vector3D& v1, PW_Vector3D& v2, PW_Vector3D& res);

inline PW_FLOAT PW_DotProduct(PW_Vector3D& v1, PW_Vector3D& v2);

void PW_MatrixProduct4D(PW_Matrix4D& m1, PW_Matrix4D& m2, PW_Matrix4D& res);

void PW_TranslationMatrix(PW_Matrix4D& m, PW_FLOAT x, PW_FLOAT y, PW_FLOAT z);

void PW_RotateByXMatrix(PW_Matrix4D& m, PW_FLOAT fAngle);

void PW_RotateByYMatrix(PW_Matrix4D& m, PW_FLOAT fAngle);

void PW_RotateByZMatrix(PW_Matrix4D& m, PW_FLOAT fAngle);

void PW_RotateByLineMatrix(PW_Matrix4D& m, PW_Line& line);

void PW_ViewMatrix(PW_Matrix4D& m, PW_Vector3D& vViewPoint, PW_Vector3D& vLookAt, PW_Vector3D& vUp);

void PW_ProjMatrix(PW_Matrix4D& m, PW_FLOAT fAngle, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar);

void PW_ViewPortMatrix(PW_Matrix4D& m, PW_FLOAT fWidth, PW_FLOAT fHeight);

inline void QuadMemSet(void* dst, int iSize, DWORD value)
{
	iSize = iSize / 4;

	__asm
	{
		mov edi, dst
			mov ecx, iSize
			mov eax, value
			rep stosd
	}
}

PW_BOOL RayInserctionPlane(PW_Vector3D& vStart, PW_Vector3D& vDelta, 
	PW_TrianglePlane& plane, PW_Vector3D& inserctionPoint, PW_Vector3D& vRefDir1
	, PW_Vector3D& vRefDir2, PW_FLOAT fRef2, PW_Vector3D& vNormal, PW_BOOL bUseVertexNormal);


//已经确保P 在三角形内
inline PW_Vector3D Interpolation( PW_Vector3D& p,  PW_Vector3D& p1,  PW_Vector3D& p2,  PW_Vector3D& p3,
	 PW_Vector3D& vNor1,  PW_Vector3D& vNor2,  PW_Vector3D& vNor3)
{
	PW_Vector3D vdir1 = p2 - p1;
	PW_Vector3D vdir2 = p3 - p1;
	PW_FLOAT u = (p.y - p1.y) / vdir1.y;
	PW_FLOAT v = (p.y - p1.y) / vdir2.y;
	PW_Vector3D vNorl = vNor1 * (1 - u) + vNor2 * u;
	PW_Vector3D vNorr = vNor1 * (1 - v) + vNor3 * v;
	PW_FLOAT x1 = p1.x + vdir1.x * u;
	PW_FLOAT x2 = p1.x + vdir2.x * v;
	PW_FLOAT uu = abs(p.x - x1) / abs(x1 - x2);
	PW_Vector3D vNorrr = vNorl * (1 - uu) + vNorr * uu;
	vNorr.Normalize();
	return vNorrr;
}
//#endif