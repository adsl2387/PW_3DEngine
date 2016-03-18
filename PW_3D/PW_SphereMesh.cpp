#include "PW_SphereMesh.h"
#include "assert.h"

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

PW_BOOL PW_SphereMesh::RayReflect(PW_LightRay& lightRay, PW_LightRay& reflectLight1, PW_LightRay& reflectLight2, PW_BOOL bCompRef)
{
	PW_FLOAT fR2 = m_fRadius2;
	PW_Vector3D vTmp = m_vPosInView - lightRay.vStart;

	PW_FLOAT fu = PW_DotProduct(vTmp, lightRay.vDir);
	PW_FLOAT fDis = vTmp.GetLen2();
	if (fu < 0 && fDis >= fR2)
	{
		return PW_FALSE;
	}
	PW_Vector3D vPs = lightRay.vStart + lightRay.vDir * fu - m_vPosInView;
	PW_FLOAT fLen2 = vPs.GetLen2();
	
	if (fLen2 >= fR2 - EPSILON)
	{
		return PW_FALSE;
	}
	
	PW_FLOAT fT2 = sqrt(fR2 - fLen2);
	PW_FLOAT fLenVdir = 1.f;
	PW_FLOAT fTu = fT2 /*/ fLenVdir*/;
	if (fDis >= fR2 + EPSILON * 10.f)
	{
		fTu = fu - fTu;
	}
	else
	{
		if (fu > 0)
		{
			fTu = fu + fTu;
		}
		else
			fTu = fu - fTu;
	}
	if (fTu < EPSILON)
	{
		return 0;
	}
	PW_Vector3D vHit = lightRay.vStart + lightRay.vDir * fTu;

	PW_Vector3D vNor = vHit - m_vPosInView;
	vNor.Normalize();
	PW_Vector3D vRef1;
	PW_FLOAT fHalf = fabs(PW_DotProduct(vNor, lightRay.vDir))  ;
	if (fDis > fR2 + EPSILON * 10.f)
	{
		vRef1 = vNor * fHalf * 2.f + lightRay.vDir;
	}
	else
	{
		vRef1 = vNor * fHalf * -2.f + lightRay.vDir;
	}
	vRef1.Normalize();
	reflectLight1.vStart = vHit;
	reflectLight1.vDir = vRef1;

	reflectLight1.cAmbient = material.cAmbient * (1 - material.fTransparency);
	reflectLight1.cDiffuse = material.cDiffuse * (1 - material.fTransparency);
	reflectLight1.cEmission = material.cEmission * (1 - material.fTransparency);
	reflectLight1.cSpecularReflection = material.cSpecularReflection * (1 - material.fTransparency);

	reflectLight1.vNormal = vNor;
	reflectLight1.vOriDir = lightRay.vDir;
	//if (fDis < fR2 + EPSILON * 10.f)//从内往外射 
	//{
	//	reflectLight1.cAmbient = material.cAmbient *(1 - material.fTransparency);
	//	reflectLight1.cDiffuse = material.cDiffuse * (1 - material.fTransparency);
	//	reflectLight1.cEmission = material.cEmission * (1 - material.fTransparency);
	//	reflectLight1.cSpecularReflection = material.cSpecularReflection * (1 - material.fTransparency);
	//}
	if (!bCompRef)
	{
		return 1;
	}
	if (material.fRef > EPSILON)//计算折射
	{
		if (fDis < fR2 + EPSILON * 10.f)//从内往外射
		{
			PW_Vector3D vIncrement = lightRay.vDir - vNor * fHalf ;
			PW_Vector3D vIncrement1 = vIncrement * material.fRef;
			PW_FLOAT fTimes2 = 1 - vIncrement1.GetLen2();
			if (fTimes2 < 0.f)//全反射
			{
				reflectLight1.cAmbient = material.cAmbient * 0.f;
				reflectLight1.cDiffuse = material.cDiffuse * 0.f;
				reflectLight1.cEmission = material.cEmission * 0.f;
				reflectLight1.cSpecularReflection = PW_COLORF(1.f, 1.f, 1.f);//material.cSpecularReflection;
				return 1;
			}
			else
			{
				PW_FLOAT fTimes = sqrt(fTimes2);
				reflectLight2.vStart = vHit;
				reflectLight2.vDir = vNor * fTimes + vIncrement1;
				reflectLight2.vDir.Normalize();
				reflectLight2.cAmbient = material.cAmbient * material.fTransparency;
				reflectLight2.cDiffuse = material.cDiffuse * material.fTransparency;
				reflectLight2.cEmission = material.cEmission * material.fTransparency;
				reflectLight2.cSpecularReflection = material.cSpecularReflection * material.fTransparency;
				reflectLight2.vNormal = vNor;
				reflectLight2.vOriDir = lightRay.vDir;
				return 2;
			}

		}
		else
		{
			PW_Vector3D vIncrement = lightRay.vDir - vNor * fHalf *-1;
			PW_Vector3D vIncrement1 = vIncrement / material.fRef;
			PW_FLOAT fTimes2 = 1 - vIncrement1.GetLen2();
			if (fTimes2 < 0.f)
			{
				reflectLight1.cAmbient = material.cAmbient * 0.f;
				reflectLight1.cDiffuse = material.cDiffuse * 0.f;
				reflectLight1.cEmission = material.cEmission * 0.f;
				reflectLight1.cSpecularReflection = PW_COLORF(1.f, 1.f, 1.f);//material.cSpecularReflection;
				//reflectLight1.cSpecularReflection = material.cSpecularReflection;
				return 1;
				//assert(fTimes2 > 0.f);
			}

			PW_FLOAT fTimes = sqrt(fTimes2);
			reflectLight2.vStart = vHit;
			reflectLight2.vDir = vNor * -fTimes + vIncrement1;
			reflectLight2.vDir.Normalize();
			reflectLight2.cAmbient = material.cAmbient * material.fTransparency;
			reflectLight2.cDiffuse = material.cDiffuse * material.fTransparency;
			reflectLight2.cEmission = material.cEmission * material.fTransparency;
			reflectLight2.cSpecularReflection = material.cSpecularReflection * material.fTransparency;
			reflectLight2.vNormal = vNor;
			reflectLight2.vOriDir = lightRay.vDir;
			return 2;

		}
	}

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