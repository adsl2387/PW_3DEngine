#pragma once
#include "PW_Obj.h"
class PW_SphereMesh :
	public PW_Mesh
{
public:
	PW_SphereMesh();
	virtual ~PW_SphereMesh();
	//virtual PW_BOOL RayInsertAABB(PW_LightRay& lightRay);

	virtual PW_BOOL RayReflect(PW_LightRay& lightRay, PW_LightRay& reflectLight1, PW_LightRay& reflectLight2, PW_BOOL bCompRef = PW_TRUE);

	virtual PW_BOOL RayInsertion(PW_Vector3D& vStart, PW_Vector3D& vDir);

	virtual void ComputeCurVertex();

	virtual PW_FLOAT GetRadius(){ return m_fRadius; }

	virtual PW_Vector3D GetPos(){ return m_vOriPos; }

	PW_Vector3D m_vPosInView;
	PW_Vector3D m_vOriPos;
	PW_FLOAT m_fRadius;
	PW_FLOAT m_fRadius2;

	void InitSphere(PW_Vector3D vPos, PW_FLOAT fRadius);
};

