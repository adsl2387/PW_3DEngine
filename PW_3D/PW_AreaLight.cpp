#include "PW_AreaLight.h"


PW_AreaLight::PW_AreaLight():
m_pMesh(NULL),
m_nShadowRayNum(100)
{
}


PW_AreaLight::~PW_AreaLight()
{
}

void PW_AreaLight::SetMesh(PW_Mesh* pMesh)
{
	m_pMesh = pMesh;
	if (pMesh)
	{
		pMesh->material.bEmissive = PW_TRUE;
		pMesh->material.cEmission = m_cDiffuse;
	}
}

void PW_AreaLight::GetShadowRays(PW_Vector3D* vStarts, PW_Vector3D* vDirs)
{

}

PW_COLORF PW_AreaLight::GetDiffuse(PW_Vector3D* pvPos /* = NULL */)
{
	if (!pvPos || !m_pMesh)
	{
		return PW_COLORF(0.f);
	}
	else
	{
		PW_Vector3D vDir = this->m_vCurDir - *pvPos;
		vDir.Normalize();
		PW_LightRay vRay, vRefRay1, vRefRay2;
		vRay.vStart = *pvPos;
		vRay.vDir = vDir;

		if (this->m_pMesh->RayReflect(vRay, vRefRay1, vRefRay2))
		{
			if (vRefRay1.vDir.IsZero())//교충
			{
				return PW_COLORF(0.f);
			}
			else
			{
				return m_cDiffuse * PW_DotProduct(vRefRay1.vDir, vRefRay1.vNormal);
			}
		}
		else
		{
			return PW_COLORF(0.f);
		}
	}
}

PW_COLORF PW_AreaLight::GetSpecular(PW_Vector3D* pvPos /* = NULL */)
{
	if (!pvPos || !m_pMesh)
	{
		return PW_COLORF(0.f);
	}
	else
	{
		PW_Vector3D vDir = m_vCurDir - *pvPos;
		vDir.Normalize();
		PW_LightRay vRay, vRefRay1, vRefRay2;
		vRay.vStart = *pvPos;
		vRay.vDir = vDir;

		if (this->m_pMesh->RayReflect(vRay, vRefRay1, vRefRay2))
		{
			if (vRefRay1.vDir.IsZero())//교충
			{
				return PW_COLORF(0.f);
			}
			else
			{
				return m_cSpecular * PW_DotProduct(vRefRay1.vDir, vRefRay1.vNormal);
			}
		}
		else
		{
			return PW_COLORF(0.f);
		}
	}
}