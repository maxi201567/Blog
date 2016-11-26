#include "WFrame.h"
#include "resource.h"
#include "Bitmap.h"
#include <iostream>

float rate = 0.5f;
POINT middle;
POINT mousep;
int nW = 0, nH = 0;
Bitmap photo;
CPEN pen;

HWND hLock;
HDC clipDC = NULL;
HBITMAP clipBM;
WNDCLASSEX WndCL;

char buf[256];

int count;

CWFrame::~CWFrame()
{
}

void CWFrame::Init(HINSTANCE hInstance,int nShowCmd, int Width, int Height, bool bWindow)
{
	
	WNDCLASSEX wndclass;
	wndclass.cbSize = sizeof(wndclass);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = "WND";
	wndclass.lpszMenuName = NULL;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;

	if (!RegisterClassEx(&wndclass))
		return;
	
	hwnd = CreateWindowEx(WS_EX_TOOLWINDOW, "WND", "WND",
		WS_BORDER | WS_CAPTION, 
		CW_USEDEFAULT, CW_USEDEFAULT, 
		Width, Height, NULL, 
		LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAIN)),
		hInstance, NULL);


	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);
}
int CWFrame::MsgLoop()
{
	while (GetMessage(&msg, NULL, 0,0))
	{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
	}
	return msg.wParam;
}
void CWFrame::Release()
{
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CHAR buf[256];
	HMENU popm;
	RECT rWnd;
	HDC hdc;
	switch (msg)
	{
	case WM_CREATE:
		GetWindowRect(hwnd, &rWnd);
		nH = GetSystemMetrics(SM_CYCAPTION) + 32;
		nW = GetSystemMetrics(SM_CXFIXEDFRAME) * 2 + 207;
		MoveWindow(hwnd, rWnd.left, rWnd.top, nW, nH, TRUE);
		SetWindowPos(hwnd, HWND_TOPMOST, rWnd.left, rWnd.top, nW, nH, SWP_SHOWWINDOW);
		RegisterHotKey(hwnd, ID_A, MOD_ALT | MOD_CONTROL, 'A');

		WndCL.cbSize = sizeof(WndCL);
		WndCL.cbClsExtra = 0;
		WndCL.cbWndExtra = 0;
		WndCL.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
		WndCL.hInstance = (HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE);
		WndCL.lpfnWndProc = NULL;
		WndCL.lpszClassName = "Extra";
		WndCL.lpszMenuName = NULL;
		WndCL.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		WndCL.hCursor = LoadCursor(NULL, IDC_HAND);
		WndCL.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		WndCL.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SHOT:
			UnregisterClass("Extra", (HINSTANCE)GetWindowLong(hLock, GWL_HINSTANCE));
			DestroyWindow(hLock);
			hLock = NULL;

			WndCL.lpfnWndProc = ShotProc;
			RegisterClassEx(&WndCL);
			hLock = CreateWindowEx(0, "Extra", NULL, WS_POPUP, 0, 0,
				GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
				hwnd, NULL, NULL, NULL);
			ShowWindow(hLock, SW_SHOW);
			UpdateWindow(hLock);
			SetFocus(hLock);
			break;

		case ID_PEN:
			UnregisterClass("Extra", (HINSTANCE)GetWindowLong(hLock, GWL_HINSTANCE));
			DestroyWindow(hLock);
			hLock = NULL;

			WndCL.lpfnWndProc = NoteProc;
			RegisterClassEx(&WndCL);
			hLock = CreateWindowEx(0, "Extra", NULL, WS_POPUP, 0, 0,
				GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
				hwnd, NULL, NULL, NULL);
			ShowWindow(hLock, SW_SHOW);
			UpdateWindow(hLock);
			SetFocus(hLock);
			break;

		case ID_EXIT:
			SendMessage(hwnd, WM_DESTROY, NULL, NULL); 
			break;

		case ID_SAVE:
			EnableMenuItem(GetMenu(hwnd), ID_SAVE, MF_DISABLED);
			if (clipDC != NULL)
				photo.SaveBitmap();
			photo.CleanUp();
			break;
		}
		break;

	case WM_HOTKEY:
		if (wParam == ID_A)
			SendMessage(hwnd, WM_COMMAND, ID_SHOT, NULL);
		break;
/*	case WM_MOUSEWHEEL:
		if (!photo.BitmapReady())break;
		rate += (short)HIWORD(wParam) > 0 ? 0.1 : -0.1;
		rate = max(rate, 0.3);
		GetWindowRect(hwnd, &rWnd);
		nW = photo.GetWidth()*rate;
		nH = photo.GetHeight()*rate;

		if (nW > GetSystemMetrics(SM_CXSCREEN) || nH > GetSystemMetrics(SM_CYSCREEN))
			break;

		MoveWindow(hwnd, middle.x - nW / 2, middle.y - nH / 2, nW, nH, TRUE);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
*/
	case WM_USER:
		if (clipDC != NULL)
		{
			GetWindowRect(hwnd, &rWnd);
			EnableMenuItem(GetMenu(hwnd), ID_SAVE, MF_ENABLED);
			hdc = ::GetDC(hwnd);
			if (photo.GetWidth() >= nW)
			{
				MoveWindow(hwnd, rWnd.left, rWnd.top, photo.GetWidth(), nH + photo.GetHeight(), FALSE);
				::BitBlt(hdc, 0, 0, photo.GetWidth(), photo.GetHeight(), clipDC, 0, 0, SRCCOPY);
			}
			else
			{
				MoveWindow(hwnd, rWnd.left, rWnd.top, nW, nH + nW / photo.GetWidth() * photo.GetHeight(), FALSE);
				::StretchBlt(hdc, 0, 0, nW, nW / photo.GetWidth() * photo.GetHeight(),
					clipDC, 0, 0, photo.GetWidth(), photo.GetHeight(), SRCCOPY);
			}
			::ReleaseDC(hwnd, hdc);
		}
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
	case WM_QUIT:
		UnregisterHotKey(hwnd, ID_A);
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK ShotProc(HWND hShot, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL click = FALSE;
	static RECT mouse;
	PAINTSTRUCT ps;
	POINT mp;
	HDC hdc;
	switch (msg)
	{
	case WM_CREATE:
		photo.ScreenShot();
		break;

	case WM_LBUTTONDOWN:
		GetCursorPos(&mp);
		mouse.left = mp.x;
		mouse.top = mp.y;
		mouse.right = mp.x;
		mouse.bottom = mp.y;
		click = TRUE;
		break;

	case WM_LBUTTONUP:
		GetCursorPos(&mp);
		mouse.right = mp.x;
		mouse.bottom = mp.y;
		clipDC = photo.GetPhotoDC();
		SendMessage(GetParent(hShot), WM_USER, 0, 0);
		SendMessage(hShot, WM_RBUTTONDOWN, NULL, NULL);
		break;

	case WM_MOUSEMOVE:
		if (click)
		{
			GetCursorPos(&mp);
			mouse.right = mp.x;
			mouse.bottom = mp.y;
		}
		photo.DrawRect(&mouse);
		hdc = ::GetDC(hShot);
		::BitBlt(hdc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), photo.GetBackDC(), 0, 0, SRCCOPY);
		::ReleaseDC(hShot, hdc);
		break;

	case WM_RBUTTONDOWN:
		click = false;
		mouse.left = mouse.right = mouse.top = mouse.bottom = 0;
		photo.DrawRect(&mouse);
		DestroyWindow(hShot);
		break;
	}

	return DefWindowProc(hShot, msg, wParam, lParam);
}
LRESULT CALLBACK NoteProc(HWND hNote, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static BOOL click = FALSE;
	static int size;
	static int color;
	static HMENU menuc, menut;

	PAINTSTRUCT ps;
	HDC hdc;
	POINT mp;

	char buf[256];

	switch (msg)
	{
	case WM_CREATE:
		photo.ScreenShot();
		size = 1; color = 0;
		menuc = GetSubMenu(LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_COLOR)), 0);
		menut = GetSubMenu(LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_TYPE)), 0);

		break;
	case WM_LBUTTONDOWN:
		SetFocus(hNote);
		click = TRUE;
		break;

	case WM_LBUTTONUP:
		click = FALSE;
		GetCursorPos(&mp);
		count = pen.Insert(mp.x, mp.y, TRUE, COLOR[color], DEGREE[size]);//左键松开
		break;

	case WM_MOUSEMOVE:
		if (click)
		{
			GetCursorPos(&mp);
			count = pen.Insert(mp.x, mp.y, FALSE, COLOR[color], DEGREE[size]);//左键按下不是断点
			pen.Display(photo.GetBackDC());
		}
		hdc = GetDC(hNote);
		::BitBlt(hdc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), photo.GetBackDC(), 0, 0, SRCCOPY);
		::ReleaseDC(hNote, hdc);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			pen.Empty();
			photo.CleanUp();
			DestroyWindow(hNote);
		}
		break;

	case WM_MOUSEWHEEL:
		if (click)break;
		GetCursorPos(&mp);
		if ((short)HIWORD(wParam) > 0)
		{
			color = ((color - 1) + 3) % 3;
			EnableMenuItem(menuc, ID_R, color == 0 ? MF_ENABLED : MF_DISABLED);
			EnableMenuItem(menuc, ID_G, color == 1 ? MF_ENABLED : MF_DISABLED);
			EnableMenuItem(menuc, ID_B, color == 2 ? MF_ENABLED : MF_DISABLED);
			TrackPopupMenu(menuc, TPM_CENTERALIGN | TPM_BOTTOMALIGN, mp.x, mp.y, 0, hNote, NULL);
		}
		if ((short)HIWORD(wParam) < 0)
		{
			size = (size + 1) % 3;
			EnableMenuItem(menut, ID_S, size == 0 ? MF_ENABLED : MF_DISABLED);
			EnableMenuItem(menut, ID_M, size == 1 ? MF_ENABLED : MF_DISABLED);
			EnableMenuItem(menut, ID_X, size == 2 ? MF_ENABLED : MF_DISABLED);
			TrackPopupMenu(menut, TPM_CENTERALIGN | TPM_TOPALIGN, mp.x, mp.y, 0, hNote, NULL);
		}
		break;
	}
	return DefWindowProc(hNote, msg, wParam, lParam);
}
