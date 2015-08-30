// PW_3D.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "PW_3D.h"
#include "PW_3DDevice.h"
#include "PW_SphereMesh.h"
#include "PW_AreaLight.h"
#include "time.h"

#define MAX_LOADSTRING 100


BOOL b_gStopRotate = FALSE;

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

PW_Mesh g_PWMesh;
PW_SphereMesh g_sphereMesh;
PW_SphereMesh g_sphereMesh1;
PW_SphereMesh g_sphereMesh2;
PW_SphereMesh g_sphereMesh3;
PW_Mesh g_PWMesh2;
PW_Mesh g_PWMesh_Ground;
PW_PerspectiveCamera g_PWCamera;
PW_Texture g_PWTexture;
PW_Texture g_PWTexture2;
PW_Light g_PWDirecLight;
PW_Light g_PWPointLight;


struct MousePos
{
	int x;
	int y;
	bool bdown;
	MousePos()
	{
		bdown = false;
	}
};

MousePos g_Mouse;

void InitScene();
void RenderScene(UINT uTime);
void Release();
#define WNDWIDTH 800
#define WNDHEIGHT 800

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PW_3D, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PW_3D));

	// 主消息循环:
	//while (GetMessage(&msg, NULL, 0, 0))

#ifdef _DEBUG
	AllocConsole();
	freopen("conout$", "w", stdout);
#endif
	//freopen("d:\\out.txt", "w", stdout);
	InitScene();
	UINT dwTime = clock();
	while (TRUE)
	{
		if (PeekMessage(&msg, NULL,0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		UINT uTime = clock();
		RenderScene(uTime - dwTime);
		dwTime = uTime;
		//g_PW3DDevice.Update();
	}
	Release();
	return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PW_3D));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PW_3D);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, WNDWIDTH, WNDHEIGHT, NULL, NULL, hInstance, NULL);

   HWND hEdit = CreateWindow("static", " hiii", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0, 0, 30, 20, hWnd, NULL, hInstance, NULL);
   if (!hWnd)
   {
      return FALSE;
   }
   g_pPW3DDevice->Create(hWnd, WNDWIDTH, WNDHEIGHT, hEdit);
   ShowWindow(hWnd, nCmdShow);
   ShowWindow(hEdit, SW_SHOW);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	PW_FLOAT d = 5;
	PW_FLOAT dr = PI / 100.f;
	INT nWidth ; // width of client area
	INT nHeight ; // height of client area
	int newx,newy;
	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case  WM_CHAR:
		
		//if (wParam == 's')
		//{
		//	b_gStopRotate = !b_gStopRotate;
		//}
		//else
		//{
		//	g_PW3DDevice.SetDrawStyle();
		//}
		
		switch (wParam)
		{
		case 'w':
			g_PWCamera.MoveDirect(d);
			break;
		case 's':
			g_PWCamera.MoveDirect(-d);
			break;
		case 'a':
			g_PWCamera.MoveLeftOrRight(d);
			break;
		case 'd':
			g_PWCamera.MoveLeftOrRight(-d);
			break;
		case 'q':
			g_PWCamera.MoveUpOrDown(-d);
			break;
		case 'e':
			g_PWCamera.MoveUpOrDown(d);
			break;
		case 'b':
			g_PWDirecLight.Switch();
			break;
		case 'n':
			g_PWPointLight.Switch();
			break;
		//case 'r':
		//	g_PWCamera.Pitch(dr);
		//	break;
		//case 'f':
		//	g_PWCamera.Pitch(-dr);
		//	break;
		//case 'x':
		//	g_PWCamera.Yaw(dr);
		//	break;
		//case 'c':
		//	g_PWCamera.Yaw(-dr);
		//	break;
		//case 'v':
		//	g_PWCamera.Roll(dr);
		//	break;
		//case 'b':
		//	g_PW3DDevice.SwitchShowAll();
		//	//g_PWCamera.Roll(-dr);
		//	break;
		case 'z':
			b_gStopRotate = !b_gStopRotate;
			break;
		case 'x':
			g_pPW3DDevice->SwitchMaterial();
			break;
		case 'g':
			g_pPW3DDevice->SwitchLight();
			break;
		case 'h':
			g_pPW3DDevice->SwitchTexture();
			break;
		case 'j':
			g_pPW3DDevice->SwitchTextFilter();
			break;
		case 'r':
			g_pPW3DDevice->SwitchRayTrace();
			break;

		case 't':
			g_pPW3DDevice->SetDrawStyle();
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		g_Mouse.bdown = true;
		g_Mouse.x = LOWORD(lParam);
		g_Mouse.y = HIWORD(lParam);
		g_pPW3DDevice->SetDrawPath(g_Mouse.x, g_Mouse.y);
		break;
	case WM_LBUTTONUP:
		g_Mouse.bdown = false;
		g_Mouse.x = LOWORD(lParam);
		g_Mouse.y = HIWORD(lParam);
		
		break;
	case WM_MOUSEMOVE:
		if (g_Mouse.bdown)
		{
			newx = LOWORD(lParam);
			newy = HIWORD(lParam);
			g_PWCamera.Yaw(PW_FLOAT(newx - g_Mouse.x) / -1000.f);
			g_PWCamera.Pitch(PW_FLOAT(newy - g_Mouse.y) / -1000.f);
			g_Mouse.x = newx;
			g_Mouse.y = newy;
		}

		break;
	case WM_SIZE:
		nWidth = LOWORD(lParam); // width of client area
		nHeight = HIWORD(lParam); // height of client area
		
		g_pPW3DDevice->Release();
		g_pPW3DDevice->Create(hWnd, nWidth, nHeight, hWnd);
		break;
	case  WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			CloseWindow(hWnd);
			PostQuitMessage(0);
		}
			break;
	case WM_DESTROY:
		
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void InitScene()
{
	if (g_PWMesh.GetVertexCount() == 0)
	{
		g_PWCamera.SetCamerInfo(PW_Vector3D(0, 30, -100), PW_Vector3D(0, 0, 0), PW_Vector3D(0, 1, 0),
			PI / 4, 1.0f, 1.f, 1000.f);
		g_pPW3DDevice->SetCamera(&g_PWCamera);

		PW_POINT3D* buffer = new PW_POINT3D[8];
		buffer[0] = PW_POINT3D(0, 0, 0, PW_RGBA(255, 0, 0));
		buffer[1] = PW_POINT3D(40, 0, 0, PW_RGBA(0, 255, 0));
		buffer[2] = PW_POINT3D(40, 40, 0, PW_RGBA(0, 0, 255));
		buffer[3] = PW_POINT3D(0, 40, 0, PW_RGBA(255, 255, 0));
		buffer[4] = PW_POINT3D(0, 0, 40, PW_RGBA(255, 0, 255));
		buffer[5] = PW_POINT3D(40, 0, 40, PW_RGBA(0, 255, 255));
		buffer[6] = PW_POINT3D(40, 40, 40, PW_RGBA(125, 125, 125));
		buffer[7] = PW_POINT3D(0, 40, 40, PW_RGBA(123, 0, 0));

		PW_POINT3D* buffer1 = new PW_POINT3D[8];
		buffer1[0] = PW_POINT3D(0, 0, 0, PW_RGBA(255, 0, 0));
		buffer1[1] = PW_POINT3D(10, 0, 0, PW_RGBA(0, 255, 0));
		buffer1[2] = PW_POINT3D(10, 10, 0, PW_RGBA(0, 0, 255));
		buffer1[3] = PW_POINT3D(0, 10, 0, PW_RGBA(255, 255, 0));
		buffer1[4] = PW_POINT3D(0, 0, 10, PW_RGBA(255, 0, 255));
		buffer1[5] = PW_POINT3D(10, 0, 10, PW_RGBA(0, 255, 255));
		buffer1[6] = PW_POINT3D(10, 10, 10, PW_RGBA(125, 125, 125));
		buffer1[7] = PW_POINT3D(0, 10, 10, PW_RGBA(123, 0, 0));

		PW_POINT3D* groundbuffer = new PW_POINT3D[4];
		groundbuffer[0] = PW_POINT3D(-100, -0, -400, PW_RGBA(255, 0, 0));
		groundbuffer[1] = PW_POINT3D(-100, -0, 400, PW_RGBA(255, 0, 0));
		groundbuffer[2] = PW_POINT3D(100, -0, 400, PW_RGBA(255, 0, 0));
		groundbuffer[3] = PW_POINT3D(100, -0, -400, PW_RGBA(255, 0, 0));
		PW_Triangle* indexbuffer = new PW_Triangle[12];
		PW_Triangle* groundindexbuffer = new PW_Triangle[2];
		int indexbbb[12][3] =
		{
			{ 1, 0, 3 },
			{ 3, 2, 1 },
			{ 1, 2, 6 },
			{ 1, 6, 5 },
			{ 5, 6, 4 },
			{ 6, 7, 4 },
			{ 4, 7, 3 },
			{ 4, 3, 0 },
			{ 3, 7, 6 },
			{ 3, 6, 2 },
			{ 4, 0, 1 },
			{ 4, 1, 5 }
		};

		int indexground[2][3] =
		{
			{ 3, 0, 1 },
			{ 1, 2, 3 }
		};
		PW_FLOAT uvs[12][6] =
		{
			{ 1, 0, 0., 0., 0, 1 },
			{ 0., 0., 1, 0., 1, 1 },

			{ 0., 1, 0., 0., 1, 0 },
			{ 0., 1, 1, 0., 1, 1 },

			{ 0., 1, 0., 0., 1, 1 },
			{ 0., 0., 1, 0., 1, 1 },

			{ 0., 1, 0., 0., 1, 0 },
			{ 0., 1, 1, 0., 1, 1 },

			{ 0., 1, 0., 0., 1, 0 },
			{ 0., 1, 1, 0., 1, 1 },

			{ 0., 1, 0., 0, 1, 0 },
			{ 0., 1, 1, 0, 1, 1 },
		};
		int numface = 12;
		for (int i = 0; i < numface; i++)
		{
			indexbuffer[i][0] = indexbbb[i][0];
			indexbuffer[i][1] = indexbbb[i][1];
			indexbuffer[i][2] = indexbbb[i][2];
			indexbuffer[i].u1 = uvs[i][0];
			indexbuffer[i].v1 = uvs[i][1];
			indexbuffer[i].u2 = uvs[i][2];
			indexbuffer[i].v2 = uvs[i][3];
			indexbuffer[i].u3 = uvs[i][4];
			indexbuffer[i].v3 = uvs[i][5];
			if (i < 2)
			{
				groundindexbuffer[i][0] = indexground[i][0];
				groundindexbuffer[i][1] = indexground[i][1];
				groundindexbuffer[i][2] = indexground[i][2];
				groundindexbuffer[i].u1 = uvs[i][0];
				groundindexbuffer[i].v1 = uvs[i][1];
				groundindexbuffer[i].u2 = uvs[i][2];
				groundindexbuffer[i].v2 = uvs[i][3];
				groundindexbuffer[i].u3 = uvs[i][4];
				groundindexbuffer[i].v3 = uvs[i][5];
			}
		}

		g_PWMesh.SetBuffer(buffer, indexbuffer, 8, numface);
		g_PWMesh2.SetBuffer(buffer1, indexbuffer, 8, numface);
		PW_Material mater;
		mater.fP = 15;
		mater.fRef = 0.f;
		mater.cAmbient = PW_COLORF(0.f, 0.f, 0.f, 0.f);
		mater.cDiffuse = PW_COLORF(0.6f, 0.6f, 0.6f);
		mater.cSpecularReflection = PW_COLORF(0.1f, 0.1f, 0.1f);

		if (g_PWTexture.LoadBitmap("tietu2.bmp"))
		{
			mater.pTexture = &g_PWTexture;
		}
		g_PWMesh.SetMaterial(mater);

		if (g_PWTexture2.LoadBitmap("tietu1.bmp"))
		{
			mater.pTexture = &g_PWTexture2;	
		}

		g_PWMesh2.SetMaterial(mater);
		g_PWMesh.UseVertexNormal(PW_TRUE);
		g_PWMesh2.UseVertexNormal(PW_TRUE);

		//ray trace init

		mater.cAmbient = PW_COLORF(0.f, 0.f, 0.f, 0.f);
		mater.cDiffuse = PW_COLORF(0.2f, 0.3f, 0.4f);
		mater.cSpecularReflection = PW_COLORF(0.3f, 0.3f, 0.3f);
		g_PWMesh_Ground.SetBuffer(groundbuffer, groundindexbuffer, 4, 2);
		g_sphereMesh.InitSphere(PW_Vector3D(0.f, 15.f, 20.f), 10.f);
		g_sphereMesh1.InitSphere(PW_Vector3D(8.f, 15.f, 40.f), 10.f);
		g_sphereMesh2.InitSphere(PW_Vector3D(0.f, 15.f, 20.f), 10.f);
		g_sphereMesh3.InitSphere(PW_Vector3D(0.f, 15.f, 20.f), 10.f);
		g_sphereMesh2.SetMaterial(mater);
		g_sphereMesh3.SetMaterial(mater);
		mater.fRef = 0.96f;
		PW_Material mtref = mater;
		mtref.fTransparency = 0.9f;
		mtref.cDiffuse = PW_COLORF(1.f, 1.f, 1.f);
		mtref.cSpecularReflection = PW_COLORF(0.03f, 0.03f, 0.03f);
		g_sphereMesh.SetMaterial(mtref);
		//mtref.fRef = 0.f;
		//mtref.fTransparency = 0.f;
		g_sphereMesh1.SetMaterial(mtref);
		PW_Material mater0;
		mater0.fP = 15.f;
		mater0.fRef = 0.f;

		g_PWMesh_Ground.UseVertexNormal(PW_TRUE);
		PW_Material groundmaterial;
		groundmaterial.cAmbient = PW_COLORF(0.01f, 0.01f, 0.01f);
		groundmaterial.cDiffuse = PW_COLORF(0.1f, 0.3f, 0.1f);
		groundmaterial.cSpecularReflection = PW_COLORF(0.f, 0.f, 0.f);
		groundmaterial.fP = 15.f;
		groundmaterial.fRef = 0.f;
		g_PWMesh_Ground.SetMaterial(groundmaterial);


		//light init
		static PW_AreaLight arealight;
		arealight.m_vPosition = PW_Vector3D(0.f, 60.f, 20.f);
		arealight.m_vDirection = PW_Vector3D(0, -1, 0);
		arealight.m_cSpecular = PW_COLORF(1.f, 1.f, 1.f);
		arealight.m_cAmbient = PW_COLORF(0.1f, 0.1f, 0.1f);
		arealight.m_cDiffuse = PW_COLORF(1.f, 1.f, 1.f);
		
		g_PWDirecLight.m_iLightType = pw_lt_directionallight;
		g_PWDirecLight.m_vPosition = PW_Vector3D(0, 100, 20);
		g_PWDirecLight.m_vDirection = PW_Vector3D(0, -1, 1);
		g_PWDirecLight.m_cSpecular = PW_COLORF(1.f, 1.f, 1.f);
		g_PWDirecLight.m_cAmbient = PW_COLORF(0.f, 0.f, 0.f);
		g_PWDirecLight.m_cDiffuse = PW_COLORF(1.f, 1.f, 1.f);
		g_pPW3DDevice->AddLight(&g_PWDirecLight);
		g_PWPointLight = g_PWDirecLight;
		g_PWPointLight.m_iLightType = pw_lt_pointlight;
		
		g_pPW3DDevice->AddLight(&g_PWPointLight);
	
		g_pPW3DDevice->SetAmbientColor(PW_COLORF(0., 0.2, 0.3));

	}
}

void RenderScene(UINT uTime)
{
	static UINT uTotalTime = 0;
	uTotalTime += uTime;
	static PW_FLOAT fr = 0;
	
	if (!b_gStopRotate)
	{
		fr += (PI / 2000 * uTime);
		if (fr >= 2 * PI)
		{
			fr -= 2 * PI;
		}
		g_pPW3DDevice->SetHelpOutputInfo(fr);
	}
	PW_Matrix4D wordmat, wordmat1;
	PW_Matrix4D rotatemat;

	if (!g_pPW3DDevice->IsRaytraceRender())
	{
		//mesh 1
		PW_RotateByXMatrix(rotatemat, PI / 6.0f);

		PW_TranslationMatrix(wordmat, -20, -20, -20);

		PW_MatrixProduct4D(rotatemat, wordmat, wordmat1);
		PW_RotateByYMatrix(rotatemat, fr);
		PW_MatrixProduct4D(rotatemat, wordmat1, wordmat);

		g_PWMesh.SetAbsTM(wordmat);

		g_pPW3DDevice->DrawMesh(g_PWMesh);

		//mesh2

		PW_RotateByXMatrix(rotatemat, 0 * PI / 6.0f);

		PW_TranslationMatrix(wordmat, -5, -5, -5);

		PW_MatrixProduct4D(rotatemat, wordmat, wordmat1);
		PW_RotateByYMatrix(rotatemat,	fr);
		PW_MatrixProduct4D(rotatemat, wordmat1, wordmat);
		PW_TranslationMatrix(wordmat1, 30, 10, 10);
		PW_MatrixProduct4D(wordmat1, wordmat, rotatemat);
		g_PWMesh2.SetAbsTM(rotatemat);

		g_pPW3DDevice->DrawMesh(g_PWMesh2);
		//	g_PW3DDevice.DrawCircle(250, 250, 100);
		//g_PW3DDevice.DrawEllipse(250, 250, 150, 100);
	}



//raytrace render
	if (g_pPW3DDevice->IsRaytraceRender())
	{
		PW_Matrix4D identymatrix;
		identymatrix.IdentityMatrix();
		//g_PW3DDevice.SetWorldTransform(identymatrix);
		g_PWMesh_Ground.SetAbsTM(identymatrix);
		g_pPW3DDevice->DrawMesh(g_PWMesh_Ground);

		PW_TranslationMatrix(wordmat1, -15, 10, 0);
		g_sphereMesh.SetAbsTM(wordmat1);

		PW_TranslationMatrix(wordmat1, 0, 15, 20);
		g_sphereMesh1.SetAbsTM(wordmat1);
		g_pPW3DDevice->DrawMesh(g_sphereMesh);
		//g_pPW3DDevice->DrawMesh(g_sphereMesh1);


		PW_TranslationMatrix(wordmat1, -10, 10, 40);
		g_sphereMesh2.SetAbsTM(wordmat1);

		PW_TranslationMatrix(wordmat1, 20, 15, 50);
		g_sphereMesh3.SetAbsTM(wordmat1);
		g_pPW3DDevice->DrawMesh(g_sphereMesh2);
		g_pPW3DDevice->DrawMesh(g_sphereMesh3);
	}
	

	PW_ViewPortMatrix(wordmat, g_pPW3DDevice->m_fWidth, g_pPW3DDevice->m_fHeight);
	g_pPW3DDevice->SetViewPortTransform(wordmat);
	g_pPW3DDevice->Render();
}

void Release()
{
	g_pPW3DDevice->Release();
	g_PWMesh.Release();
}
