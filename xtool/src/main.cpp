#include "WFrame.h"
#include "D3D.h"


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmd, int nShowCmd)
{
	CWFrame MainWnd;
	MainWnd.Init(hInstance, nShowCmd, 60, 80, TRUE);
	MainWnd.MsgLoop();
	MainWnd.Release();
}