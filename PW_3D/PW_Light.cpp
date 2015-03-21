#include "PW_Light.h"
#include "PW_3DDevice.h"

PW_COLORF PW_Light::GetDiffuse(PW_Vector3D* vPos /* = NULL */)
{
	return m_cDiffuse;
}

PW_COLORF PW_Light::GetSpecular(PW_Vector3D* vPos /* = NULL */)
{
	return m_cSpecular;
}

PW_COLORF PW_Light::RayTraceColor(PW_Vector3D& vPos, PW_INT nMeshIndex, PW_Vector3D& vNormal, PW_Vector3D& vOriDir)
{
	PW_Vector3D lightdir;
	PW_COLORF fRet;
	if (m_iLightType == pw_lt_spotlight)
	{
	}
	else if (m_iLightType == pw_lt_pointlight)
	{
		lightdir = m_vCurDir - vPos;
		lightdir.Normalize();
	}
	else
	{
		lightdir = m_vCurDir;
	}
	vector<PW_Mesh*>& pMeshs = g_pPW3DDevice->GetAllMeshs();
	for (int ii = 0; ii < pMeshs.size(); ii++)
	{
		if (ii != nMeshIndex &&!pMeshs[ii]->material.bEmissive && pMeshs[ii]->RayInsertion(vPos, lightdir))
		{
			return fRet;
		}
	}
	PW_FLOAT fRes = PW_DotProduct(lightdir, vNormal);
	if (fRes > EPSILON)
	{
		fRet = fRet + m_cDiffuse * fRes;
	}

	//相机总是在0,0,0
	PW_Vector3D vP = vOriDir * -1;
	vP.Normalize();
	vP = vP + lightdir;
	vP.Normalize();
	PW_FLOAT fTmp = PW_DotProduct(vNormal, vP);
	fTmp = pow(fTmp, pMeshs[nMeshIndex].material.fP);
	if (fTmp > 0)
	{
		fRet = fRet + m_cSpecular * fTmp;
	}
	fRet += m_cAmbient;
	return fRet;
}