#include "PW_Math.h"

class PW_Camera
{
public:
	PW_Camera();
	~PW_Camera();

	void Init(PW_Vector3D vPos, PW_Vector3D vLookAt ,PW_Vector3D vUp);

	//ǰ���ƶ�
	void MoveDirect(PW_FLOAT d);

	//�����ƶ�
	void MoveLeftOrRight(PW_FLOAT d);

	//�����ƶ�
	void MoveUpOrDown(PW_FLOAT d);

	//���Ӹ��Ӷ��� ������ X����ת
	void Pitch(PW_FLOAT d);

	//���� Y����ת
	void Yaw(PW_FLOAT d);

	//���� Z����ת
	void Roll(PW_FLOAT d);

	PW_Matrix4D GetViewMat(){ return m_viewMat; }
private:
	PW_Matrix4D m_viewMat;
	PW_Vector3D m_vPos;
	PW_Vector3D m_vUp;
	PW_Vector3D m_vLookAt;
};
