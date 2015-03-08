#include "PW_SphereMesh.h"


PW_SphereMesh::PW_SphereMesh():
m_vPosInView(0.f),
m_fRadius(0.f)
{
}


PW_SphereMesh::~PW_SphereMesh()
{
}

void PW_SphereMesh::InitSphere(PW_Vector3D vPos, PW_FLOAT fRadius)
{
	if (fRadius < 0.f)
	{
		return;
	}
	m_vOriPos = vPos;
	m_fRadius = fRadius;
	m_fRadius2 = m_fRadius * m_fRadius;
	
}

PW_BOOL PW_SphereMesh::RayInsertion(PW_Vector3D& vStart, PW_Vector3D& vDir)
{
	PW_FLOAT fR2 = m_fRadius2;
	PW_Vector3D vTmp = m_vPosInView - vStart;

	PW_FLOAT fu = PW_DotProduct(vTmp, vDir);
	if (fu < -EPSILON && vTmp.GetLen2() > fR2)
	{
		return PW_FALSE;
	}
	PW_Vector3D vPs = vStart + vDir * fu - m_vPosInView;
	PW_FLOAT fLen2 = vPs.GetLen2();

	if (fLen2 > fR2)
	{
		return PW_FALSE;
	}
	else
	{
		return PW_TRUE;
	}

}

PW_BOOL PW_SphereMesh::RayReflect(PW_LightRay& lightRay, PW_LightRay& reflectLight1, PW_LightRay& reflectLight2)
{
	PW_FLOAT fR2 = m_fRadius2;
	PW_Vector3D vTmp = m_vPosInView - lightRay.vStart;

	PW_FLOAT fu = PW_DotProduct(vTmp, lightRay.vDir);
	if (fu < -EPSILON && vTmp.GetLen2() > fR2)
	{
		return PW_FALSE;
	}
	PW_Vector3D vPs = lightRay.vStart + lightRay.vDir * fu - m_vPosInView;
	PW_FLOAT fLen2 = vPs.GetLen2();
	
	if (fLen2 > fR2)
	{
		return PW_FALSE;
	}
	
	PW_FLOAT fT2 = sqrt(fR2 - fLen2);
	PW_FLOAT fLenVdir = 1.f;
	PW_FLOAT fTu = fT2 /*/ fLenVdir*/;
	if (fu < -EPSILON)
	{
		fTu = fu + fTu;
	}
	else
	{
		fTu = fu - fTu;
	}

	PW_Vector3D vHit = lightRay.vStart + lightRay.vDir * fTu;

	PW_Vector3D vNor = vHit - m_vPosInView;
	vNor.Normalize();
	PW_Vector3D vRef1;
	PW_FLOAT fHalf = PW_DotProduct(vNor, lightRay.vDir)  * -2.f;
	vRef1 = vNor * fHalf - lightRay.vDir;
	vRef1.Normalize();
	reflectLight1.vStart = vHit;
	reflectLight1.vDir = vRef1;
	reflectLight1.cAmbient = material.cAmbient;
	reflectLight1.cDiffuse = material.cDiffuse;
	reflectLight1.cEmission = material.cEmission;
	reflectLight1.cSpecularReflection = material.cSpecularReflection;
	reflectLight1.vNormal = vNor;
	reflectLight1.vOriDir = lightRay.vDir;
	return PW_TRUE;
}

void PW_SphereMesh::ComputeCurVertex()
{
	PW_FLOAT fRadius = m_fRadius;
	PW_Vector4D vTmp = m_vOriPos.MatrixProduct(m_absoluteTM);
	vTmp.NoneHomogeneous();
	m_vPosInView.x = vTmp.x;
	m_vPosInView.y = vTmp.y;
	m_vPosInView.z = vTmp.z;
	this->curAABB = PW_AABB(PW_Vector3D(m_vPosInView.x - fRadius, m_vPosInView.y - fRadius, m_vPosInView.z - fRadius),
		PW_Vector3D(m_vPosInView.x + fRadius, m_vPosInView.y + fRadius, m_vPosInView.z + fRadius));
}