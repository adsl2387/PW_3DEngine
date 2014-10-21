//#ifndef PW_MATH
//#define PW_MATH
#pragma once
#include <windows.h>
#include <math.h>
typedef float PW_FLOAT;
typedef int PW_INT;
typedef double PW_DOUBLE;
typedef unsigned char PW_BYTE;

#define PI 3.1415926535898
#define EPSILON 0.0000001
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

	PW_Vector4D MatrixProduct(PW_Matrix4D& mat);

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


	PW_Vector4D(PW_FLOAT fx = 0.f, PW_FLOAT fy=0.f, PW_FLOAT fz=0.f, PW_FLOAT fw = 1.f)
		:x(fx),y(fy),z(fz),w(fw)
	{}

	PW_Vector4D(PW_Vector3D& vec)
		:x(vec.x),y(vec.y),z(vec.z),w(1.f)
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

//#endif