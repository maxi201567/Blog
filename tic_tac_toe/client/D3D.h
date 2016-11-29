#include <d3d9.h>
#include <C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Include\\d3dx9.h>
#include "C:\\Users\\Maxi\\Desktop\\_INPUT\\CAMERA.h"
#include "C:\\Users\\Maxi\\Desktop\\_INPUT\\INPUT.h"
#include "C:\\Users\\Maxi\\Desktop\\_DDRAW\\LOAD.h"
#include "resource.h"

#include <Windows.h>
#include <stdio.h>


#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Lib\\x86\\d3dx9.lib")

#pragma once

#define DC_T05 D3DCOLOR_ARGB(128, 255,255,255)//半透明
#define DC_T1  D3DCOLOR_ARGB(255, 255,255,255)//不透明

extern char form[3][3];
extern WCHAR textbuf[256];
extern BOOL bReady[2];
extern BOOL bIng;
extern BOOL bRed;//是否是红色
extern BOOL bTurn;//是否轮到你
extern BOOL bType;//true是圈，false是叉
extern BOOL bRecv;
extern int oot;
extern char bWin;

static D3DXVECTOR3 vIcon[12];
static RECT rIcon[7];
static RECT rform[3][3];
static D3DXMATRIX scale1, scale2;//放大倍数

class CD3D
{

private:
	HWND _hwnd;
	IDirect3D9 *_interface;
	IDirect3DDevice9 *_device;
	KeyBoard _keydev;
	Mouse _moudev;
	ID3DXFont *_font;

	ID3DXSprite *_sback;
	IDirect3DTexture9 *_tback, *_ticon;

	POINT _pmou;

	float _tApp, _tRun, _tLast;
	float _msfps, _fps;
	int _nfps;

public:
	void Init(HWND hwnd, D3DPRESENT_PARAMETERS *d3dpp);
	void Release();
	void Display();
};

void GetMousePosition(POINT *position, HWND hwnd);
void DrawModel(HWND hwnd, POINT *pos, LPDIRECT3DTEXTURE9 texture, Mouse *mouse, LPD3DXSPRITE sprite);
