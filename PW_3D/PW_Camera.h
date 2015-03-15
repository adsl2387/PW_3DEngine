#pragma once
#include "PW_Math.h"

class PW_Camera
{
public:
	PW_Camera();
	~PW_Camera();

	void Init(PW_Vector3D vPos, PW_Vector3D vLookAt ,PW_Vector3D vUp, PW_FLOAT fFOV, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar);

	//前后移动
	void MoveDirect(PW_FLOAT d);

	//左右移动
	void MoveLeftOrRight(PW_FLOAT d);

	//上下移动
	void MoveUpOrDown(PW_FLOAT d);

	//仰视俯视度数 弧度制 X轴旋转
	void Pitch(PW_FLOAT d);

	//方向 Y轴旋转
	void Yaw(PW_FLOAT d);

	//翻滚 Z轴旋转
	void Roll(PW_FLOAT d);

	PW_Matrix4D GetViewMat(){ return m_viewMat; }

	PW_Matrix4D GetProjMat(){ return m_projMat; }
private:
	PW_Matrix4D m_viewMat;
	PW_Vector3D m_vPos;
	PW_Vector3D m_vUp;
	PW_Vector3D m_vLookAt;
	PW_Matrix4D m_projMat;
};

