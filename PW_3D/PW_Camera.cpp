#include "PW_Camera.h"

PW_CameraBase::PW_CameraBase()
{
}

PW_CameraBase::~PW_CameraBase()
{
}

void PW_CameraBase::MoveDirect(PW_FLOAT d)
{
	PW_Matrix4D m,m1;
	PW_TranslationMatrix(m, 0, 0, -d);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}

void PW_CameraBase::MoveLeftOrRight(PW_FLOAT d)
{
	PW_Matrix4D m, m1;
	PW_TranslationMatrix(m, -d, 0, 0);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}

void PW_CameraBase::MoveUpOrDown(PW_FLOAT d)
{
	PW_Matrix4D m, m1;
	PW_TranslationMatrix(m, 0, -d, 0);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}

void PW_CameraBase::Yaw(PW_FLOAT d)
{
	PW_Matrix4D m, m1;
	PW_RotateByYMatrix(m, -d);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}

void PW_CameraBase::Pitch(PW_FLOAT d)
{
	PW_Matrix4D m, m1;
	PW_RotateByXMatrix(m, -d);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}

void PW_CameraBase::Roll(PW_FLOAT d)
{
	PW_Matrix4D m, m1;
	PW_RotateByZMatrix(m, -d);
	PW_MatrixProduct4D(m, m_viewMat, m1);
	m_viewMat = m1;
}


PW_PerspectiveCamera::PW_PerspectiveCamera()
{
}

PW_PerspectiveCamera::~PW_PerspectiveCamera()
{
}

void PW_PerspectiveCamera::SetCamerInfo(PW_Vector3D vPos, PW_Vector3D vLookAt, PW_Vector3D vUp, PW_FLOAT fFOV, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar)
{
	PW_ViewMatrix(m_viewMat, vPos, vLookAt, vUp);
	PW_PersProjMatrix(m_projMat, fFOV, fRate, fNear, fFar);
}

PW_OrthoCamera::PW_OrthoCamera():
m_fScale(1.f)
{

}

PW_OrthoCamera::~PW_OrthoCamera()
{

}

void PW_OrthoCamera::MoveDirect(PW_FLOAT d)
{
	m_fScale -= d / 100.f;
	if (m_fScale < 0.5f)
	{
		m_fScale = 0.5f;
	}
	PW_Matrix4D mat;
	mat.IdentityMatrix();
	mat[0][0] = 1.f / m_fScale;
	mat[1][1] = 1.f / m_fScale;
	PW_Matrix4D res;
	PW_MatrixProduct4D(m_projMat, mat, res);
	m_projMat = res;
}

void PW_OrthoCamera::SetCamerInfo(PW_Vector3D vPos, PW_Vector3D vLookAt, PW_Vector3D vUp, PW_FLOAT fFOV, PW_FLOAT fRate, PW_FLOAT fNear, PW_FLOAT fFar)
{
	PW_ViewMatrix(m_viewMat, vPos, vLookAt, vUp);
	PW_OrthoProjMatrix(m_projMat, fFOV, fRate, fNear, fFar);
	m_fScale = 1.f;
}