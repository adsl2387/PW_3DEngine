#pragma once

#include "PW_Math.h"
#include "PW_Obj.h"

enum PW_LIGHT_TYPE
{
	pw_lt_pointlight,
	pw_lt_directionallight,
	pw_lt_spotlight,
	pw_lt_arealight,//ÇøÓò¹â
};

class PW_Light: public PW_Object
{

public:
	PW_FLOAT m_fAttenuation0;
	PW_FLOAT m_fAttenuation1;
	PW_FLOAT m_fAttenuation2;
	PW_COLORF m_cDiffuse;
	PW_COLORF m_cAmbient;
	PW_COLORF m_cSpecular;
	PW_INT m_iLightType;
	PW_Vector3D m_vPosition;
	PW_Vector3D m_vDirection;
	PW_FLOAT m_fRange;
	PW_Vector3D m_vCurDir;
	PW_BOOL m_bOn;

	PW_Light()
	{
		m_fAttenuation0 = 0;
		m_fAttenuation1 = 0;
		m_fAttenuation2 = 0;
		m_bOn = PW_TRUE;
	}
	PW_Light(PW_FLOAT p1, PW_FLOAT p2, PW_FLOAT p3, PW_COLORF cDif, PW_COLORF cAmb, PW_COLORF cSpe, PW_INT lightType,
		PW_Vector3D pos, PW_Vector3D dir, PW_FLOAT fRan)
		:m_fAttenuation0(p1), m_fAttenuation1(p2), m_fAttenuation2(p3),
		m_cDiffuse(cDif), m_cAmbient(cAmb), m_cSpecular(cSpe), m_iLightType(lightType),
		m_vPosition(pos), m_vDirection(dir), m_fRange(fRan)
	{}

	virtual PW_COLORF GetDiffuse(PW_Vector3D* vPos );

	virtual PW_COLORF GetSpecular(PW_Vector3D* vPos );

	virtual PW_COLORF RayTraceColor(PW_Vector3D& vPos, PW_LightRay& lightRay, PW_INT nMeshIndex ,PW_Vector3D& vNormal, PW_Vector3D& vOriDir);

	virtual PW_Vector3D GetPos(){ return m_vPosition; }

	virtual void Switch(){ m_bOn = !m_bOn; }
};