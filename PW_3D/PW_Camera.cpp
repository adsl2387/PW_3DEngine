#include "PW_Camera.h"

PW_Camera::PW_Camera()
{
}

PW_Camera::~PW_Camera()
{
}

void PW_Camera::Init(PW_Vector3D vPos, PW_Vector3D vLookAt, PW_Vector3D vUp, PW_FLOAT fFOV, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar)
{
	PW_ViewMatrix(m_viewMat, vPos, vLookAt, vUp);
	PW_ProjMatrix(m_projMat, fFOV, fRate, fNear, fFar);
}

void PW_Camera::MoveDirect(PW_FLOAT d)
{
	PW_Matrix4D m,m1;
	PW_TranslationMatrix(m, 0, 0, -d);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}

void PW_Camera::MoveLeftOrRight(PW_FLOAT d)
{
	PW_Matrix4D m, m1;
	PW_TranslationMatrix(m, -d, 0, 0);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}

void PW_Camera::MoveUpOrDown(PW_FLOAT d)
{
	PW_Matrix4D m, m1;
	PW_TranslationMatrix(m, 0, -d, 0);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}

void PW_Camera::Yaw(PW_FLOAT d)
{
	PW_Matrix4D m, m1;
	PW_RotateByYMatrix(m, -d);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}

void PW_Camera::Pitch(PW_FLOAT d)
{
	PW_Matrix4D m, m1;
	PW_RotateByXMatrix(m, -d);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}

void PW_Camera::Roll(PW_FLOAT d)
{
	PW_Matrix4D m, m1;
	PW_RotateByZMatrix(m, -d);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}