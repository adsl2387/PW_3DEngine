#pragma once
#include "PW_Obj.h"
#include "PW_Light.h"
class PW_AreaLight :public PW_Light
{
public:
	PW_AreaLight();
	virtual ~PW_AreaLight();
	virtual PW_COLORF GetDiffuse(PW_Vector3D* vPos = NULL);
	virtual PW_COLORF GetSpecular(PW_Vector3D* vPos = NULL);
	virtual void SetMesh(PW_Mesh* pMesh);
	virtual PW_Mesh* GetMesh(){ return m_pMesh; }
	void GetShadowRays(PW_Vector3D* vStarts, PW_Vector3D* vDirs);
protected:
	PW_Mesh* m_pMesh;
	PW_INT m_nShadowRayNum;
};

