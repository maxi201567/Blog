#include <Windows.h>
#include "D3D.h"
#include "resource.h"
#include <winsock.h>
#include <string>
#pragma comment(lib, "ws2_32.lib")

#pragma once


class CWFrame
{
private:
	HWND hwnd;
	MSG msg;
	CD3D d3d;
			 
public:
	void Init(HINSTANCE hInstance, int nShowCmd, int Width, int Height, bool bWindow);//����һ��������
	int MsgLoop();
	void Release();

};

void InitGame();
void UpdateText(HWND hwnd);
DWORD WINAPI NetThread(LPVOID lParam);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);//��Ϣѭ��
BOOL CALLBACK IpProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);//�Ի�����

