#include "WFrame.h"

char form[3][3];
WCHAR textbuf[256];
BOOL bReady[2];
BOOL bIng;
BOOL bRed;
BOOL bTurn;
BOOL bType;
BOOL bRecv;//是否要接收数据
int oot;
char bWin;

BOOL bConnect;//是否连接成功

HWND gwnd;

std::string IP;//服务器地址
SOCKADDR_IN aClient;
SOCKET sClient;

void CWFrame::Init(HINSTANCE hInstance,int nShowCmd, int Width, int Height, bool bWindow)
{
	InitGame();
	bIng = FALSE;//是否在游戏中
	bReady[0] = bReady[1] = FALSE;//准备按键

	lstrcpy(textbuf, L"离线中。。。");//开始是离线的	
	//**********************window初始化
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = L"WND";
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;

	if (!RegisterClassEx(&wndclass))
		return;

	hwnd = CreateWindow(L"WND", L"井 - MAXI",
		WS_CAPTION,
		CW_USEDEFAULT, CW_USEDEFAULT, 
		Width, Height,
		NULL, LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU)), 
		hInstance, NULL);
	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	RECT temp0, temp1;
	GetWindowRect(hwnd, &temp0);
	GetClientRect(hwnd, &temp1);
	MoveWindow(hwnd, temp0.left, temp0.top, 
		Width + (temp0.right - temp1.right) + (temp1.left - temp0.left),
		Height + (temp0.bottom - temp1.bottom) + (temp1.top - temp0.top), 
		TRUE);

	gwnd = hwnd;
	//****************D3D初始化
	D3DPRESENT_PARAMETERS d3dpp;
	::ZeroMemory(&d3dpp, sizeof(D3DPRESENT_PARAMETERS));
	d3dpp.Windowed = bWindow;//*
	d3dpp.BackBufferWidth = Width;
	d3dpp.BackBufferHeight = Height;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.EnableAutoDepthStencil = false;
	d3dpp.AutoDepthStencilFormat = D3DFMT_UNKNOWN;
	d3dpp.MultiSampleQuality =0;
	d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;//立即刷新
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;//*

	d3d.Init(hwnd, &d3dpp);

	//*****************Socket初始化
	IP = "127.0.0.1";
	WSADATA wdata;
	aClient.sin_family = AF_INET;
	aClient.sin_port = htons(5150);
	aClient.sin_addr.S_un.S_addr = inet_addr(IP.c_str());
	WSAStartup(MAKEWORD(2, 2), &wdata);
	sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
}
int CWFrame::MsgLoop()
{
	::PeekMessage(&msg, 0, 0, 0, PM_REMOVE);
	while (msg.message != WM_QUIT)
	{
		if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
			d3d.Display();
	}
	return msg.wParam;
}
void CWFrame::Release()
{
	d3d.Release();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDR_IP: 
			DialogBox((HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE), MAKEINTRESOURCE(IDD_DIALOG), hwnd, IpProc);
			break;
		case IDR_READY:
			EnableMenuItem(GetMenu(hwnd), IDR_READY, MF_DISABLED);
			EnableMenuItem(GetMenu(hwnd), IDR_EXIT, MF_DISABLED);
			DrawMenuBar(hwnd);
			bReady[0] = TRUE;
			send(sClient, "r", 1, 0);
			break;

		case IDR_EXIT:	
			::PostQuitMessage(0); break;
		}
		break;  
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	case WM_QUIT:
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}
BOOL CALLBACK IpProc(HWND hwndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		SetDlgItemTextA(hwndDlg, IDC_EDIT, IP.c_str());
		break;
	case WM_COMMAND:
		char buf[16];
		switch (LOWORD(wParam))
		{
		case IDOK:		
			GetDlgItemTextA(hwndDlg, IDC_EDIT, buf, 16); IP = buf; 
			EndDialog(hwndDlg, 0);
			aClient.sin_addr.S_un.S_addr = inet_addr(IP.c_str());
			UpdateText(GetParent(hwndDlg));
			break;

		case IDCANCEL:  
			EndDialog(hwndDlg, 0);   
			break;
		}

	}
	return false;	
}

void UpdateText(HWND hwnd)
{
	if (connect(sClient, (SOCKADDR*)&aClient, sizeof(aClient)) == 0)
	{
		lstrcpy(textbuf, L"已连接服务器");

		CreateThread(NULL, 0, NetThread, NULL, 0, NULL);

		EnableMenuItem(GetMenu(hwnd), IDR_READY, MF_ENABLED);
		EnableMenuItem(GetMenu(hwnd), IDR_IP, MF_DISABLED);
		DrawMenuBar(hwnd);
	}
	else
		lstrcpy(textbuf, L"离线中。。。");
}

DWORD WINAPI NetThread(LPVOID lParam)
{
	char buf[2];
	while (1)
	{
		//颜色
		recv(sClient, buf, 1, 0);
		bRed = (strncmp(buf, "R", 1) == 0);
		//符号
		recv(sClient, buf, 1, 0);
		bType = (strncmp(buf, "o", 1) == 0);
		//准备
		recv(sClient, buf, 1, 0);
		bReady[1] = (strncmp(buf, "Y", 1) == 0);
		//是否开始
		recv(sClient, buf, 1, 0);
		bIng = (strncmp(buf, "s", 1) == 0);

		while (1)
		{
			if (bIng)
			{
				recv(sClient, buf, 1, 0);//接收输赢
				bWin = buf[0];
				recv(sClient, buf, 1, 0);
				bTurn = (strncmp(buf, "C", 1) == 0);
				oot = 5;
				if (bTurn)
				{
					recv(sClient, form[0], 3, 0);
					recv(sClient, form[1], 3, 0);
					recv(sClient, form[2], 3, 0);
					bRecv = FALSE;
					oot = 5;
					while (bTurn);
					//等待bTurn = F
					send(sClient, form[0], 3, 0);
					send(sClient, form[1], 3, 0);
					send(sClient, form[2], 3, 0);
					bRecv = TRUE;
				}
			}
			if (bWin != 0)break;
		}

		bIng = FALSE;
		bReady[0] = FALSE;
		Sleep(3000);//3s后刷新到下一局
		EnableMenuItem(GetMenu(gwnd), IDR_READY, MF_ENABLED);
		EnableMenuItem(GetMenu(gwnd), IDR_EXIT, MF_ENABLED);
		DrawMenuBar(gwnd);
		InitGame();
		wsprintf(textbuf, L"请准备！");
	}
	return 0;
}

void InitGame()
{
	memset(form, -1, sizeof(form));//初始化棋阵
	ZeroMemory(textbuf, sizeof(textbuf));//初始化文字缓冲区
    bReady[1] = FALSE;//准备按键
	bWin = 0;
	bRed = TRUE;
	bTurn = FALSE;
	bRecv = TRUE;

}