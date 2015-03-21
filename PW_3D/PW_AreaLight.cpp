#include "PW_AreaLight.h"


PW_AreaLight::PW_AreaLight():
m_pMesh(NULL),
m_nShadowRayNum(10)
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

void PW_AreaLight::GetShadowRays(PW_Vector3D& vPoint, PW_Vector3D* vStarts, PW_Vector3D* vDirs)
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
		PW_Vector3D vDir;
		vDir = *pvPos - this->m_vCurDir ;
		PW_FLOAT fStep = m_pMesh->GetRadius() / m_nShadowRayNum;
		PW_Vector3D vStart;
		PW_FLOAT fR = 0;
		PW_Vector3D vTmp = vDir + PW_Vector3D(vDir.x, 2 * vDir.y, 3 * vDir.z);
		PW_Matrix4D mTmp, mTm;
		PW_ViewMatrix(mTmp, this->m_vCurDir, vTmp + this->m_vCurDir, vDir);
		//PW_Vector4D v1 = PW_Vector3D(0, 1, 0).MatrixProduct(mTmp);
		PW_Vector4D vY = PW_Vector4D(0, 1, 0, 0);
		PW_Vector4D vZ = PW_Vector4D(0, 0, 1, 0);
		PW_Vector3D vAt = PW_Vector3D(0.f).MatrixProduct(mTmp);
		vY.MatrixProduct(mTmp);
		vZ.MatrixProduct(mTmp);
		PW_Vector3D vYAxis(vY);
		PW_Vector3D vZAxis(vZ);
		PW_ViewMatrix(mTm, vAt, vAt + vZAxis, vYAxis);

		//PW_Vector3D vUp = PW_Vector3D(PW_Vector3D(0, 1, 0).MatrixProduct(mTmp)) - PW_Vector3D( PW_Vector3D(0).MatrixProduct(mTmp));
		//PW_ViewMatrix(mTm, this->m_vCurDir * -1.f, PW_Vector3D(0.f), vUp);
		PW_COLORF fValue;
		for (int i = 0; i < m_nShadowRayNum;i++)
		{
			fR += fStep;
			for (int j = 0; j < 8;j++) //为了减少计算  只取8个方向
			{
				vStart.x = cos(PI * j / 4.f) * fR;
				vStart.z = sin(PI * j / 4.f) * fR;
				vStart.y = 0.f;
				PW_Vector3D vS = vStart.MatrixProduct(mTm);
				vDir = vS - *pvPos;
				vDir.Normalize();
				PW_LightRay vRay, vRefRay1, vRefRay2;
				vRay.vStart = *pvPos;
				vRay.vDir = vDir;

				if (this->m_pMesh->RayReflect(vRay, vRefRay1, vRefRay2))
				{
					if (vRefRay1.vDir.IsZero())//背面
					{
						//return PW_COLORF(0.f);
					}
					else
					{
						fValue += m_cDiffuse * PW_DotProduct(vRefRay1.vDir, vRefRay1.vNormal);
					}
				}
				else
				{
					//return PW_COLORF(0.f);
				}
			}
		}
		
		return fValue / 8 / m_nShadowRayNum;
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
			if (vRefRay1.vDir.IsZero())//背面
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