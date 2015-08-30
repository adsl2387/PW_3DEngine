#pragma once
#include "PW_Math.h"

class PW_CameraBase
{
public:
	PW_CameraBase();
	virtual ~PW_CameraBase();

	virtual void SetCamerInfo(PW_Vector3D vPos, PW_Vector3D vLookAt, PW_Vector3D vUp, PW_FLOAT fFOV, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar) = 0;

	//前后移动
	virtual void MoveDirect(PW_FLOAT d);

	//左右移动
	virtual void MoveLeftOrRight(PW_FLOAT d);

	//上下移动
	virtual void MoveUpOrDown(PW_FLOAT d);

	//仰视俯视度数 弧度制 X轴旋转
	virtual void Pitch(PW_FLOAT d);

	//方向 Y轴旋转
	virtual void Yaw(PW_FLOAT d);

	//翻滚 Z轴旋转
	virtual void Roll(PW_FLOAT d);

	PW_Matrix4D GetViewMat(){ return m_viewMat; }

	PW_Matrix4D GetProjMat(){ return m_projMat; }

protected:
	PW_Matrix4D m_viewMat;
	PW_Vector3D m_vPos;
	PW_Vector3D m_vUp;
	PW_Vector3D m_vLookAt;
	PW_Matrix4D m_projMat;
};

class PW_PerspectiveCamera:public PW_CameraBase
{
public:
	PW_PerspectiveCamera();
	virtual ~PW_PerspectiveCamera();

	virtual void SetCamerInfo(PW_Vector3D vPos, PW_Vector3D vLookAt ,PW_Vector3D vUp, PW_FLOAT fFOV, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar);



};

class PW_OrthoCamera :public PW_CameraBase
{
public:
	PW_OrthoCamera();

	virtual ~PW_OrthoCamera();

	virtual void SetCamerInfo(PW_Vector3D vPos, PW_Vector3D vLookAt, PW_Vector3D vUp, PW_FLOAT fFOV, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar);

	//前后移动
	virtual void MoveDirect(PW_FLOAT d);

protected:
	PW_FLOAT m_fScale;
};