#include "PW_Math.h"

inline void PW_CrossProduct(PW_Vector3D& v1, PW_Vector3D& v2, PW_Vector3D& res)
{
	res.x = v1.y * v2.z - v1.z * v2.y;
	res.y = v1.z * v2.x - v1.x * v2.z;
	res.z = v1.x * v2.y - v1.y * v2.x;
}

PW_FLOAT PW_DotProduct(PW_Vector3D& v1, PW_Vector3D& v2)
{
	PW_FLOAT fres = 0.f;
	fres += v1.x * v2.x;
	fres += v1.y * v2.y;
	fres += v1.z * v2.z;
	return fres;
}

void PW_MatrixProduct4D(PW_Matrix4D& m1, PW_Matrix4D& m2, PW_Matrix4D& res)
{
	res.Zero();
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			for (int k = 0; k < 4; ++k)
			{
				res[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
}

void PW_TranslationMatrix(PW_Matrix4D& m, PW_FLOAT x, PW_FLOAT y, PW_FLOAT z)
{
	m.IdentityMatrix();
	m[0][3] = x;
	m[1][3] = y;
	m[2][3] = z;
}

void PW_RotateByXMatrix(PW_Matrix4D& m, PW_FLOAT fAngle)
{
	m.Zero();
	m[0][0] = 1;
	m[3][3] = 1;
	m[1][1] = cos(fAngle);
	m[1][2] = sin(fAngle);
	m[2][1] = -m[1][2];
	m[2][2] = m[1][1];
}

void PW_RotateByYMatrix(PW_Matrix4D& m, PW_FLOAT fAngle)
{
	m.Zero();
	m[1][1] = 1;
	m[3][3] = 1;
	m[0][0] = cos(fAngle);
	m[0][2] = -sin(fAngle);
	m[2][0] = -m[0][2];
	m[2][2] = m[0][0];
}

void PW_RotateByZMatrix(PW_Matrix4D& m, PW_FLOAT fAngle)
{
	m.Zero();
	m[2][2] = 1;
	m[3][3] = 1;
	m[0][0] = cos(fAngle);
	m[0][1] = sin(fAngle);
	m[1][0] = -m[0][1];
	m[1][1] = m[0][0];
}

void PW_RotateByLineMatrix(PW_Matrix4D& m, PW_Line& line)
{

}

void PW_ViewMatrix(PW_Matrix4D& m, PW_Vector3D& vViewPoint, PW_Vector3D& vLookAt, PW_Vector3D& vUp)
{
	PW_Vector3D zAxis = vLookAt - vViewPoint;
	zAxis.Normalize();
	PW_Vector3D xAxis;
	PW_CrossProduct(vUp, zAxis, xAxis);
	xAxis.Normalize();
	PW_Vector3D yAxis;
	PW_CrossProduct(zAxis, xAxis, yAxis);
	yAxis.Normalize();
	m.Zero();
	m[0][0] = xAxis.x;
	m[0][1] = xAxis.y;
	m[0][2] = xAxis.z;
	m[0][3] = -PW_DotProduct(vViewPoint, xAxis);
	m[1][0] = yAxis.x;
	m[1][1] = yAxis.y;
	m[1][2] = yAxis.z;
	m[1][3] = -PW_DotProduct(vViewPoint, yAxis);
	m[2][0] = zAxis.x;
	m[2][1] = zAxis.y;
	m[2][2] = zAxis.z;
	m[2][3] = -PW_DotProduct(vViewPoint, zAxis);
	m[3][3] = 1;
}

void PW_ProjMatrix(PW_Matrix4D& m, PW_FLOAT fAngle, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar)
{
	m.Zero();
	PW_FLOAT sx, sy;
	
	sx = tan(fAngle / 2) ;
	sx = 1 / sx ;
	//sx *= fRate;
	sy = sx;
	m[0][0] =  sx * fRate;
	m[1][1] = m[0][0];
	m[2][2] = (fNear + fFar) / (fFar - fNear);
	m[2][3] = 2 * fFar * fNear / (fNear - fFar);
	m[3][2] = 1;
}

void PW_ViewPortMatrix(PW_Matrix4D& m, PW_FLOAT fWidth, PW_FLOAT fHeight)
{
	m.Zero();
	PW_FLOAT fr = fHeight / fWidth;
	m[0][0] = fWidth / 2 * fr ;
	m[0][3] = fWidth / 2;
	m[1][1] = -fHeight / 2;
	m[1][3] = fHeight / 2;
	m[2][2] = 1 / 2.f;
	m[2][3] = 1 / 2.f;
	m[3][3] = 1;
}

PW_Vector4D PW_Vector3D::MatrixProduct(PW_Matrix4D& mat)
{
	PW_Vector4D v4d(*this);
	v4d.MatrixProduct(mat);
	return v4d;
}

PW_Vector3D::PW_Vector3D(PW_Vector4D& v4d)
{
	x = v4d.x;
	y = v4d.y;
	z = v4d.z;
	//pwColor = color;
}

void PW_Vector3D::operator=(PW_Vector4D& v4d)
{
	x = v4d.x;
	y = v4d.y;
	z = v4d.z;
}
void PW_Vector4D::MatrixProduct(PW_Matrix4D& mat)
{
	PW_FLOAT fx = mat[0][0] * x + mat[0][1] * y + mat[0][2] * z + mat[0][3] * w;
	PW_FLOAT fy = mat[1][0] * x + mat[1][1] * y + mat[1][2] * z + mat[1][3] * w;
	PW_FLOAT fz = mat[2][0] * x + mat[2][1] * y + mat[2][2] * z + mat[2][3] * w;
	PW_FLOAT fw = mat[3][0] * x + mat[3][1] * y + mat[3][2] * z + mat[3][3] * w;
	x = fx;
	y = fy;
	z = fz;
	w = fw;
}

PW_BOOL RayInserctionPlane(PW_Vector3D& vStart, PW_Vector3D& vDelta, PW_TrianglePlane& plane, PW_Vector3D& inserctionPoint
	, PW_Vector3D& vRefDir1, PW_Vector3D& vRefDir2, PW_FLOAT fRef2, PW_Vector3D& vNormalll)
{
	vDelta.Normalize();
	PW_Vector3D e1 = plane.p2 - plane.p1;
	PW_Vector3D e2 = plane.p3 - plane.p2;
	PW_Vector3D e3 = plane.p1 - plane.p3;
	PW_Vector3D vNorm;
	PW_CrossProduct(e1, e2, vNorm);
	PW_Vector3D vp = plane.p1 - vStart;
	//return PW_FALSE;
	if (PW_DotProduct(vDelta, vNorm) >= 0.f)
	{
		//射线照到的是背面
		return PW_FALSE;
	}
	PW_FLOAT u;
	PW_FLOAT dot1 = PW_DotProduct(vp, vNorm);
	PW_FLOAT dot2 = PW_DotProduct(vDelta, vNorm);
	u = dot1 / dot2;
	if (u < 0)
	{
		return PW_FALSE;
	}
	PW_Vector3D inserp = vStart + vDelta * u;
	//
	PW_Vector3D d1 = inserp - plane.p1;
	PW_Vector3D d2 = inserp - plane.p2;
	PW_Vector3D d3 = inserp - plane.p3;
	PW_Vector3D cross1, cross2, cross3;
	PW_CrossProduct(e1, d1, cross1);
	PW_CrossProduct(e2, d2, cross2);
	PW_CrossProduct(e3, d3, cross3);
	if (PW_DotProduct(cross1, cross2) < 0 || PW_DotProduct(cross2, cross3) < 0 ||
		PW_DotProduct(cross1, cross3) < 0)
	{
		return PW_FALSE;
	}
	vNorm.Normalize();
	vNormalll = vNorm;
	vRefDir1 = vDelta - vNorm * 2 * (PW_DotProduct(vDelta, vNorm));
	vRefDir1.Normalize();
	
	if (fRef2 > 0)
	{
		PW_Vector3D vvTmp = (vNorm * -1 - vDelta) * fRef2;
		vRefDir2 = (vNorm * -1) + vvTmp;
		vRefDir2.Normalize();
	}
	
	inserctionPoint = inserp;
	return PW_TRUE;
}
