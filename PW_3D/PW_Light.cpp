#include "PW_Light.h"

PW_COLORF PW_Light::GetDiffuse(PW_Vector3D* vPos /* = NULL */)
{
	return m_cDiffuse;
}

PW_COLORF PW_Light::GetSpecular(PW_Vector3D* vPos /* = NULL */)
{
	return m_cSpecular;
}