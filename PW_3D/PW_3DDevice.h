#pragma once
//#ifndef PW_3DDEVICE
//#define PW_3DDEVICE

#include <Windows.h>
#include <vector>
#include "PW_Camera.h"
#include "PW_define.h"

#include "PW_Obj.h"
#include "PW_Light.h"
using namespace std;
enum PW_DS
{
	wireframe = 1,
	solid,
	pw_dscount,
};

extern PW_BOOL gBFlag ;


PW_Vector4D VertexShader(PW_POINT3D& in);

PW_COLOR PixelShader(PW_POINT3D& in);

PW_Vector4D ShadowMapVS(PW_POINT3D& in);

PW_COLOR ShadowMapPS(PW_POINT3D& in);

PW_Vector4D ReceiveShodowVS(PW_POINT3D& in);

PW_COLOR ReceiveShadowPS(PW_POINT3D& in);

class PW_3DDevice
{
public:

	static PW_3DDevice* GetSingleton();
	bool Create(HWND hWnd, int iWidth, int iHeight, HWND hEdit);
	void Release();
	
	void Render();

	void SwitchRayTrace(){ m_bRayTrace = !m_bRayTrace; }

	void SetRayTrace(){ m_bRayTrace = true; }

	void SetDrawPath(int x, int y){
		m_nDrawX = x;
		m_nDrawY = y;
	}
	void Update();
	void DrawMesh(PW_Mesh& mesh);

	void DrawEllipse(PW_FLOAT x, PW_FLOAT y, PW_FLOAT a, PW_FLOAT b, PW_COLOR pwColor = PW_RGB(255, 255, 255));

	void DrawCircle(PW_FLOAT x, PW_FLOAT y, PW_FLOAT r, PW_COLOR pwColor = PW_RGB(255,255,255));

	void SetViewPortTransform(PW_Matrix4D& m){ m_viewportMatrix = m; }

	void SetHelpOutputInfo(PW_FLOAT fRotate){ m_fRotate = fRotate; }

	void SetDrawStyle(){ m_ds = pw_dscount - m_ds; }

	void ComputeLight(PW_POINT3D& point, PW_Matrix4D& viewMat);

	void AddLight(PW_Light* pLight);

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
			SetTexture(m_material.pTexture);
		}
		
	}

	inline PW_FLOAT GetValueOfShadowZBuffer(PW_INT x, PW_INT y)
	{
		if (x < 0 || x >= m_iWidth || y < 0 || y >= m_iHeight)
			return 99999.f;
		return m_pShadowZBuffer[y * m_iWidth + x]; }

	inline void ViewPortClamp(PW_Vector4D& p1);

	PW_BOOL IsRaytraceRender(){ return m_bRayTrace; }

	void SwitchMaterial(){m_bUseMaterial = !m_bUseMaterial;}

	void SwitchLight(){ m_bUseLight = !m_bUseLight; }

	void SwitchTexture(){ m_bUseTexture = !m_bUseTexture; }

	void RenderShadowMap(PW_Light* pLight);

	void SetTexture(PW_Texture* text)
	{
		if (!text)
		{
			m_bUseTexture = false;
		}
		else
		{
			m_texture = text;
			m_bUseTexture = true;
		}
	}

	vector<PW_Mesh*>& GetAllMeshs(){ return m_pMeshs; }
	
	void SwitchShowAll(){m_bShowAll = !m_bShowAll;}

	void EnableLight(bool bUse){m_bUseLight = bUse;}
	void SwitchTextFilter(){ m_bUseBiliner = !m_bUseBiliner; }
	void SetCamera(PW_CameraBase* car){ m_pCamera = car; }

	//根据顶点颜色插值画线
	void DrawLine2D(PW_POINT3D point1, PW_POINT3D point2, int isolid = 1, PW_BOOL bEnableZ = PW_TRUE);

	void DrawLine3D(PW_POINT3D point1, PW_POINT3D point2);

	PW_CameraBase* GetCamera(){ return m_pCamera; }

	PW_INT GetViewPortHeight(){ return m_iHeight; }

	PW_CameraBase* GetShadowMapCamera(){ return m_pShadowMapCamera; }

	PW_Matrix4D GetViewportMatrix(){ return m_viewportMatrix; }

	PW_Vector4D GetOriPos(PW_FLOAT x, PW_FLOAT y, PW_FLOAT z);

	DWORD* GetColorBuffer(){ return m_pBitBuffer; }

	void SetColorBuffer(DWORD* pBuffer){ m_pBitBuffer = pBuffer; }

	PW_FLOAT* GetZBuffer(){ return m_pZBuffer; }

	void SetZBuffer(PW_FLOAT* pBuffer){ m_pZBuffer = pBuffer; }

	DWORD GetRenderState(){ return m_dwRenderState; }

	DWORD SetRenderState(DWORD dwValue){ m_dwRenderState = dwValue; }

	PW_Texture* GetTexture(){ return m_texture; }

	PW_BOOL UseBilinerTex(){ return m_bUseBiliner; }

	//屏幕坐标到观察坐标
	inline void GetViewPos(PW_Vector3D& vScreen, PW_Vector3D& vRes)
	{
		PW_FLOAT xx, yy, zz;
		xx = vScreen.x;
		yy = vScreen.y;
		zz = vScreen.z;
		//PW_Vector3D vRes;
		vRes.z = GetViewZ(zz);
		PW_Matrix4D projMatrix = this->m_pCamera->GetProjMat();
		vRes.x = (xx - this->m_viewportMatrix[0][3]) / this->m_viewportMatrix[0][0] * vRes.z / projMatrix[0][0];
		vRes.y = (yy - this->m_viewportMatrix[1][3]) / this->m_viewportMatrix[1][1] * vRes.z / projMatrix[1][1];

		//return vRes;
	}

	PW_3DDevice();
	~PW_3DDevice();
	PW_FLOAT m_fWidth;
	PW_FLOAT m_fHeight;
protected:

	PW_Vector4D (*fnVertexShader)(PW_POINT3D& in);

	PW_COLOR (*fnPixelShader)(PW_POINT3D& in);

	void RayTrace();

	void RenderScene();

	PW_COLORF RayTraceRec(PW_RayTraceNode* pNode, PW_INT nDepth, PW_INT& nOutTotalD, PW_FLOAT& dis);

	PW_COLORF RayComputerLight(PW_RayTraceNode* pNode);

	void Clear(PW_COLOR pwcolor, PW_FLOAT pwzbuffer);

	//根据纹理
	void DrawLineTexture(PW_POINT3D point1, PW_POINT3D point2, int isolid = 1);
	void DrawTriPrimitive(PW_POINT3D point1, PW_POINT3D point2, PW_POINT3D point3, PW_COLOR color, int ds = wireframe);
	void DrawTriangle(PW_POINT3D point1, PW_POINT3D point2, PW_POINT3D point3);
	void DrawTriangle(PW_Vector4D point1, PW_Vector4D point2, PW_Vector4D point3);

	void OutputHelpInfo();


	
	inline PW_FLOAT GetValueOfZBuffer(PW_INT x, PW_INT y){ 
		return m_pZBuffer[y * m_iWidth + x];
	}


	inline void SetValueOfZBuffer(PW_INT x, PW_INT y, PW_FLOAT v){ m_pZBuffer[y * m_iWidth + x] = v; }
	inline void SetValueOfCBuffer(PW_INT x, PW_INT y, PW_COLOR v)
	{
		y = m_iHeight - 1 - y;
		m_pBitBuffer[y * m_iWidth + x] = v;
	}

	void UpdateCurLight();

	//投影矩阵和屏幕矩阵的特殊性直接算
	//屏幕Z到观察Z
	inline PW_FLOAT GetViewZ(PW_FLOAT z)
	{
		//PW_FLOAT z1 = (z + z) - 1.0f;
		//PW_Matrix4D projMatrix = this->m_pCamera->GetProjMat();
		//z1 = projMatrix[2][3] / (z1 - projMatrix[2][2]);
		//return z1;
		return this->m_pCamera->GetViewZ(z);
	}

	//观察Z到屏幕Z
	inline PW_FLOAT GetViewPortZ(PW_FLOAT z)
	{
		//PW_Matrix4D projMatrix = this->m_pCamera->GetProjMat();
		//PW_FLOAT z1 = projMatrix[2][3] / z + projMatrix[2][2];
		//z1 = z1 / 2.0f + 0.5f;
		//return z1;
		return this->m_pCamera->GetViewPortZ(z);
	}



	inline void SetPixel(PW_INT x, PW_INT y, PW_COLOR pwColor)
	{
		y = m_iHeight - y - 1;
		if (y >= m_iHeight || y < 0 || x >= m_iWidth || x < 0)
		{
			return;
		}
		this->m_pBitBuffer[y * m_iWidth + x] = pwColor;
	}
private:
	PW_CameraBase* m_pCamera;
	int m_nCurNodePos;
	HWND m_hWnd;
	int m_iWidth;
	int m_iHeight;

	HBITMAP m_hBitmap;
	DWORD* m_pBitBuffer;
	PW_FLOAT* m_pZBuffer;

	PW_FLOAT* m_pShadowZBuffer;

	HWND m_hEdit;
	DWORD m_lastTick;
	DWORD m_iFps;
	DWORD m_iFcount;

	//PW_Matrix4D m_worldMatrix;
	//PW_Matrix4D m_viewMatrix;
	//PW_Matrix4D m_projMatrix;
	PW_Matrix4D m_viewportMatrix;
	PW_FLOAT m_fRotate;
	PW_Vector4D* m_v4dBuffer;
	PW_Vector3D* m_vNormalsBuffer;
	PW_INT* m_nIndexBuffer;
	PW_FLOAT* m_fUBuffer;
	PW_FLOAT* m_fVBuffer;
	PW_INT m_curV4DPos;
	PW_INT m_curIndexPos;

	PW_Material m_material;
	bool m_bUseMaterial;
	int m_4dBuffersize;
	int m_ds;
	vector<PW_Light*> m_vLights;
	PW_COLORF m_Ambient;
	PW_Texture* m_texture;

	PW_FLOAT m_fMaxZ;
	PW_FLOAT m_fMinZ;
	PW_INT m_nMaxDepth;
	bool m_bUseLight;
	bool m_bUseTexture;
	bool m_bUseBiliner;
	vector<PW_Mesh*> m_pMeshs;
	PW_BOOL m_bRayTrace;
	int m_bShow;
	bool m_bShowAll;
	int m_bWrite;
	//VE

	//draw line
	int m_nDrawX;
	int m_nDrawY;
	vector<PW_POINT3D> m_vecPath;

	DWORD m_dwRenderState;
	PW_CameraBase* m_pShadowMapCamera;

};
extern PW_3DDevice* g_pPW3DDevice;
//#endif