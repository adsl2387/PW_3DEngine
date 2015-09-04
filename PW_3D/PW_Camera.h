#pragma once
#include "PW_Math.h"

class PW_CameraBase
{
public:
	PW_CameraBase();
	virtual ~PW_CameraBase();

	virtual void SetCamerInfo(PW_Vector3D vPos, PW_Vector3D vLookAt, PW_Vector3D vUp, PW_FLOAT fFOV, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar) = 0;

	//ǰ���ƶ�
	virtual void MoveDirect(PW_FLOAT d);

	//�����ƶ�
	virtual void MoveLeftOrRight(PW_FLOAT d);

	//�����ƶ�
	virtual void MoveUpOrDown(PW_FLOAT d);

	//���Ӹ��Ӷ��� ������ X����ת
	virtual void Pitch(PW_FLOAT d);

	//���� Y����ת
	virtual void Yaw(PW_FLOAT d);

	//���� Z����ת
	virtual void Roll(PW_FLOAT d);

	PW_Matrix4D GetViewMat(){ return m_viewMat; }

	PW_Matrix4D GetProjMat(){ return m_projMat; }

	PW_Matrix4D GetInverseViewMat(){ return m_inverseViewMat; }


	PW_BOOL IsOrthoCamera(){ return m_bOrtho; }

	virtual PW_FLOAT GetViewPortZ(PW_FLOAT fViewZ) = 0;

	virtual PW_FLOAT GetViewZ(PW_FLOAT fScreenZ) = 0;
protected:
	PW_Matrix4D m_viewMat;
	PW_Vector3D m_vPos;
	PW_Vector3D m_vUp;
	PW_Vector3D m_vLookAt;
	PW_Matrix4D m_projMat;

	PW_Matrix4D m_inverseViewMat;
	PW_BOOL		m_bOrtho;
};

class PW_PerspectiveCamera:public PW_CameraBase
{
public:
	PW_PerspectiveCamera();
	virtual ~PW_PerspectiveCamera();

	virtual void SetCamerInfo(PW_Vector3D vPos, PW_Vector3D vLookAt ,PW_Vector3D vUp, PW_FLOAT fFOV, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar);

	virtual PW_FLOAT GetViewZ(PW_FLOAT fScreenZ);

	virtual PW_FLOAT GetViewPortZ(PW_FLOAT fViewZ);

};

class PW_OrthoCamera :public PW_CameraBase
{
public:
	PW_OrthoCamera();

	virtual ~PW_OrthoCamera();

	virtual void SetCamerInfo(PW_Vector3D vPos, PW_Vector3D vLookAt, PW_Vector3D vUp, PW_FLOAT fFOV, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar);

	//ǰ���ƶ�
	virtual void MoveDirect(PW_FLOAT d);

	virtual PW_FLOAT GetViewZ(PW_FLOAT fScreenZ);

	virtual PW_FLOAT GetViewPortZ(PW_FLOAT fViewZ);
protected:
	PW_FLOAT m_fScale;
	PW_Matrix4D m_matBaseProj;
};