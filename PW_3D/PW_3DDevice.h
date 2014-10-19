#pragma once
//#ifndef PW_3DDEVICE
//#define PW_3DDEVICE

#include <Windows.h>
#include <vector>
#include "PW_Camera.h"

#include "PW_Obj.h"
using namespace std;
enum PW_DS
{
	wireframe = 1,
	solid,
	pw_dscount,
};

class PW_3DDevice
{
public:
	bool Create(HWND hWnd, int iWidth, int iHeight, HWND hEdit);
	void Release();
	
	void Update();
	void DrawMesh(PW_Mesh& mesh);

	void SetWorldTransform(PW_Matrix4D& m){ m_worldMatrix = m; }
	void SetViewTransform(PW_Matrix4D& m){ m_viewMatrix = m; }
	void SetProjTransform(PW_Matrix4D& m){ m_projMatrix = m; }
	void SetViewPortTransform(PW_Matrix4D& m){ m_viewportMatrix = m; }

	void SetHelpOutputInfo(PW_FLOAT fRotate){ m_fRotate = fRotate; }
	void SetDrawStyle(){ m_ds = pw_dscount - m_ds; }
	void ComputeLight(PW_POINT3D& point, PW_Matrix4D& viewMat);

	void AddLight(PW_Light light){ m_vLights.push_back(light); }

	void SetAmbientColor(PW_COLORF amb){ m_Ambient = amb; }

	void SetMaterial(PW_Material* material)
	{
		if (!material)
		{
			m_bUseMaterial = false;
		}
		else
		{
			m_material = *material;
			m_bUseMaterial = true;
		}
		
	}
	void SetCamera(PW_Camera* car){ m_Camera = car; }
	PW_3DDevice();
	~PW_3DDevice();
protected:
	void Clear(PW_COLOR pwcolor, PW_FLOAT pwzbuffer);
	//void DrawPoint(PW_POINT point, PW_COLOR pwcolor);
	//void DrawLine(PW_POINTF point1, PW_POINTF point2, PW_COLOR color, int isolid = 1);

	void DrawLine(PW_POINT3D point1, PW_POINT3D point2, int isolid = 1);

	void DrawTriPrimitive(PW_POINT3D point1, PW_POINT3D point2, PW_POINT3D point3, PW_COLOR color, int ds = wireframe);
	void DrawTriangle(PW_POINT3D point1, PW_POINT3D point2, PW_POINT3D point3);
	void Present();
	
private:
	PW_Camera* m_Camera;
	HWND m_hWnd;
	int m_iWidth;
	int m_iHeight;
	HBITMAP m_hBitmap;
	DWORD* m_pBitBuffer;
	PW_FLOAT* m_pZBuffer;
	HWND m_hEdit;
	DWORD m_lastTick;
	DWORD m_iFps;
	DWORD m_iFcount;

	PW_Matrix4D m_worldMatrix;
	PW_Matrix4D m_viewMatrix;
	PW_Matrix4D m_projMatrix;
	PW_Matrix4D m_viewportMatrix;
	PW_FLOAT m_fRotate;
	PW_Vector4D* m_v4dBuffer;
	PW_Vector3D* m_vNormalsBuffer;
	PW_Material m_material;
	bool m_bUseMaterial;
	int m_4dBuffersize;
	int m_ds;
	vector<PW_Light> m_vLights;
	PW_COLORF m_Ambient;

	PW_FLOAT m_fMaxZ;
	PW_FLOAT m_fMinZ;
};

//#endif