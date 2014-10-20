// PW_3D.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "PW_3D.h"
#include "PW_3DDevice.h"

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
PW_3DDevice g_PW3DDevice;
PW_Mesh g_PWMesh;
PW_Camera g_PWCamera;
PW_Texture g_PWTexture;

void RenderScene();
void Release();
#define WNDWIDTH 1200
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
		RenderScene();
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
   g_PW3DDevice.Create(hWnd, WNDWIDTH, WNDHEIGHT, hEdit);
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
			g_PWCamera.MoveLeftOrRight(-d);
			break;
		case 'd':
			g_PWCamera.MoveLeftOrRight(d);
			break;
		case 'q':
			g_PWCamera.MoveUpOrDown(-d);
			break;
		case 'e':
			g_PWCamera.MoveUpOrDown(d);
			break;
		case 'r':
			g_PWCamera.Pitch(dr);
			break;
		case 'f':
			g_PWCamera.Pitch(-dr);
			break;
		case 'x':
			g_PWCamera.Yaw(dr);
			break;
		case 'c':
			g_PWCamera.Yaw(-dr);
			break;
		case 'v':
			g_PWCamera.Roll(dr);
			break;
		case 'b':
			g_PWCamera.Roll(-dr);
			break;
		case 'z':
			b_gStopRotate = !b_gStopRotate;
			break;
		case 'g':
			g_PW3DDevice.SwitchLight();
			break;
		case 'h':
			g_PW3DDevice.SwitchTexture();
			break;
		default:
			g_PW3DDevice.SetDrawStyle();
			break;
		}
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

void RenderScene()
{
	static PW_FLOAT fr = 0;
	if (g_PWMesh.GetVertexCount() == 0)
	{
		if (g_PWTexture.LoadBitmap("d:\\tietu.bmp"))
		{
			g_PW3DDevice.SetTexture(&g_PWTexture);
		}
		
		PW_POINT3D* buffer = new PW_POINT3D[8];
		buffer[0] = PW_POINT3D(0, 0, 0, PW_RGBA(255, 0, 0));
		buffer[1] = PW_POINT3D(40, 0, 0, PW_RGBA(0, 255, 0));
		buffer[2] = PW_POINT3D(40, 40, 0, PW_RGBA(0, 0, 255));
		buffer[3] = PW_POINT3D(0, 40, 0, PW_RGBA(255, 255, 255));
		buffer[4] = PW_POINT3D(0, 0, 40, PW_RGBA(255, 0, 255));
		buffer[5] = PW_POINT3D(40, 0, 40, PW_RGBA(0, 255, 255));
		buffer[6] = PW_POINT3D(40, 40, 40, PW_RGBA(125, 125, 125));
		buffer[7] = PW_POINT3D(0, 40, 40, PW_RGBA(123, 0, 0));
		
		PW_Triangle* indexbuffer = new PW_Triangle[12];
		int indexbbb[12][3] = 
		{
			{0,3,1},
			{3,2,1},
			{1,2,5},
			{2,6,5},
			{5,6,4},
			{6,7,4},
			{4,7,0},
			{7,3,0},
			{3,7,2},
			{7,6,2},
			{4,0,5},
			{0,1,5}
		};
		PW_FLOAT uvs[12][6]=
		{
			{ 0.1, 0.9, 0.1, 0.1, 0.9, 0.9 },
			{ 0.1, 0.1, 0.9, 0.1, 0.9, 0.9 },
			{ 0.1, 0.9, 0.1, 0.1, 0.9, 0.9 },
			{ 0.1, 0.1, 0.9, 0.1, 0.9, 0.9 },
			{ 0.1, 0.9, 0.1, 0.1, 0.9, 0.9 },
			{ 0.1, 0.1, 0.9, 0.1, 0.9, 0.9 },
			{ 0.1, 0.9, 0.1, 0.1, 0.9, 0.9 },
			{ 0.1, 0.1, 0.9, 0.1, 0.9, 0.9 },
			{ 0.1, 0.9, 0.1, 0.1, 0.9, 0.9 },
			{ 0.1, 0.1, 0.9, 0.1, 0.9, 0.9 },
			{ 0.1, 0.9, 0.1, 0.1, 0.9, 0.9 },
			{ 0.1, 0.1, 0.9, 0.1, 0.9, 0.9 },
		};
		for (int i = 0; i < 12;i++)
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
		}
	
		g_PWMesh.SetBuffer(buffer, indexbuffer, 8, 12);
	
		PW_Material mater;
		mater.fP = 1;
		mater.cAmbient = PW_COLORF(0., 0., 0., 0.);
		mater.cDiffuse = PW_COLORF(1,1, 1);
		mater.cSpecularReflection = PW_COLORF(0.0, 0.0, 0.);
		g_PW3DDevice.SetMaterial(&mater);
		PW_Light light;
		light.iLightType = pw_lt_directionallight;
		light.vDirection = PW_Vector3D(0, 0, 1);
		light.cSpecular = PW_COLORF(0.5, 0.5, 0.5);
		light.cAmbient = PW_COLORF(0., 0., 0.);
		light.cDiffuse = PW_COLORF(1, 1, 1);
		g_PW3DDevice.AddLight(light);
		g_PWCamera.Init(PW_Vector3D(0, 0, -100), PW_Vector3D(0, 0, 0), PW_Vector3D(0, 1, 0));
		g_PW3DDevice.SetCamera(&g_PWCamera);
		g_PW3DDevice.SetAmbientColor(PW_COLORF(0.0, 0.0, 0.0));
	}
	if (!b_gStopRotate)
	{
		fr += PI / 5000;
		if (fr >= 2 * PI)
		{
			fr -= 2 * PI;
		}
		//fr = 0.2013 * 2 * PI;
		//fr = 0.7299 * 2 * PI;
		//fr = 0;
		g_PW3DDevice.SetHelpOutputInfo(fr);
	}
	
	//mesh 1
	PW_Matrix4D rotatemat;
	PW_RotateByXMatrix(rotatemat, PI / 4.0f);
	PW_Matrix4D wordmat, wordmat1;
	PW_TranslationMatrix(wordmat, -20, -20, -20);

	PW_MatrixProduct4D(rotatemat, wordmat, wordmat1);
	PW_RotateByYMatrix(rotatemat, fr);
	PW_MatrixProduct4D(rotatemat, wordmat1, wordmat);

	
	g_PW3DDevice.SetWorldTransform(wordmat);
	PW_ViewMatrix(wordmat, PW_Vector3D(0, 0, -100), PW_Vector3D(0, 0, 0), PW_Vector3D(0, 1, 0));
	g_PW3DDevice.SetViewTransform(wordmat);
	PW_ProjMatrix(wordmat, PI / 2, 1, 1, 1000);
	g_PW3DDevice.SetProjTransform(wordmat);
	PW_ViewPortMatrix(wordmat, WNDWIDTH, WNDHEIGHT);
	g_PW3DDevice.SetViewPortTransform(wordmat);
	
//	g_PW3DDevice.DrawMesh(g_PWMesh);
	
	//mesh2
	
	PW_RotateByXMatrix(rotatemat, PI / 4.0f);
	
	PW_TranslationMatrix(wordmat, -20, -20, -20);

	PW_MatrixProduct4D(rotatemat, wordmat, wordmat1);
	PW_RotateByYMatrix(rotatemat, fr * 10);
	PW_MatrixProduct4D(rotatemat, wordmat1, wordmat);
	PW_TranslationMatrix(wordmat1, 20, 20, 40);
	PW_MatrixProduct4D(wordmat1, wordmat, rotatemat);

	g_PW3DDevice.SetWorldTransform(rotatemat);
	PW_ViewMatrix(wordmat, PW_Vector3D(0, 0, -100), PW_Vector3D(0, 0, 0), PW_Vector3D(0, 1, 0));
	g_PW3DDevice.SetViewTransform(wordmat);
	PW_ProjMatrix(wordmat, PI / 2, 1, 1, 1000);
	g_PW3DDevice.SetProjTransform(wordmat);
	PW_ViewPortMatrix(wordmat, WNDWIDTH, WNDHEIGHT);
	g_PW3DDevice.SetViewPortTransform(wordmat);
	//g_PW3DDevice.SetAmbientColor(PW_COLORF(0., 0, 0.5));
	g_PW3DDevice.DrawMesh(g_PWMesh);
	
	g_PW3DDevice.Update();
}

void Release()
{
	g_PW3DDevice.Release();
	g_PWMesh.Release();
}