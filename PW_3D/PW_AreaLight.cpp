#include "PW_AreaLight.h"
#include "PW_3DDevice.h"

PW_AreaLight::PW_AreaLight():
m_pMesh(NULL),
m_nShadowRayNum(20),
m_nShadowRayCircleNum(16)
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
			fR += fStep;
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

PW_COLORF PW_AreaLight::RayTraceColor(PW_Vector3D& vPos, PW_INT nMeshIndex, PW_Vector3D& vNormal, PW_Vector3D& vOriDir)
{
	PW_Vector3D vDir;
	vDir = vPos - this->m_vCurDir;
	vDir.Normalize();
	PW_FLOAT fStep = m_pMesh->GetRadius() / m_nShadowRayNum;
	PW_Vector3D vStart;
	PW_FLOAT fR = 0;
	PW_Vector3D vTmp = vDir + PW_Vector3D(vDir.x, 2 * vDir.y, 3 * vDir.z);
	PW_Matrix4D mTmp, mTm;
	PW_ViewMatrix(mTmp, this->m_vCurDir, vPos, vTmp);
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
	vector<PW_Mesh*>& pMeshs = g_pPW3DDevice->GetAllMeshs();
	PW_BOOL bOss = PW_FALSE;
	for (int i = 0; i < m_nShadowRayNum; i++)
	{

		for (int j = 0; j < m_nShadowRayCircleNum; j++) //为了减少计算  只取8个方向
		{
			vStart.x = cos(PI * j / m_nShadowRayCircleNum * 2.f) * fR;
			vStart.y = sin(PI * j / m_nShadowRayCircleNum * 2.f) * fR;
			vStart.z = 0.f;
			PW_Vector3D vS = vStart.MatrixProduct(mTm);
			vDir = vS - vPos;
			vDir.Normalize();
			PW_LightRay vRay, vRefRay1, vRefRay2;
			vRay.vStart = vPos;
			vRay.vDir = vDir;
#ifdef AREATEST
			if (gBFlag)
			{
				PW_Vector4D v4P = vPos.MatrixProduct(g_pPW3DDevice->GetCamera()->GetProjMat());
				v4P.NoneHomogeneous();
				PW_Vector4D v4E = vS.MatrixProduct(g_pPW3DDevice->GetCamera()->GetProjMat());
				v4E.NoneHomogeneous();

				v4P.MatrixProduct(g_pPW3DDevice->GetViewportMatrix());
				v4E.MatrixProduct(g_pPW3DDevice->GetViewportMatrix());

				PW_POINT3D pS, pE;
				pS.x = v4P.x;
				pS.y = v4P.y;
				pS.z = v4P.z;
				pE.x = v4E.x;
				pE.y = v4E.y;
				pE.z = v4E.z;

				g_pPW3DDevice->DrawLine2D(pS, pE);
			}
#endif // AREATEST
			PW_BOOL bInsert = PW_FALSE;

			for (int ii = 0; ii < pMeshs.size(); ii++)
			{
				if (ii != nMeshIndex &&!pMeshs[ii]->material.bEmissive && pMeshs[ii]->RayInsertion(vPos, vDir))
				{
					bInsert = PW_TRUE;
					break;
				}
			}
			if (!bInsert)
			{
				if (this->m_pMesh->RayReflect(vRay, vRefRay1, vRefRay2))
				{
					if (vRefRay1.vDir.IsZero())//背面
					{
						//return PW_COLORF(0.f);
					}
					else
					{
						bOss = PW_TRUE;
						//fValue += m_cDiffuse * PW_DotProduct(vRefRay1.vDir, vRefRay1.vNormal);
						PW_FLOAT fRes = PW_DotProduct(vDir, vNormal);
						if (fRes > EPSILON)
						{
							fValue = fValue + m_cDiffuse * fRes * pMeshs[nMeshIndex]->material.cDiffuse;

						}

						//相机总是在0,0,0
						PW_Vector3D vP = vOriDir * -1;
						vP.Normalize();
						vP = vP + vDir;
						vP.Normalize();
						PW_FLOAT fTmp = PW_DotProduct(vNormal, vP);
						fTmp = pow(fTmp, pMeshs[nMeshIndex]->material.fP);
						if (fTmp > 0)
						{
							fValue = fValue + m_cSpecular * fTmp * pMeshs[nMeshIndex]->material.cSpecularReflection;
						}
					}
				}
			}
		}

		fR += fStep;
	}

	return fValue / m_nShadowRayCircleNum / m_nShadowRayNum + m_cAmbient* pMeshs[nMeshIndex]->material.cAmbient;
}