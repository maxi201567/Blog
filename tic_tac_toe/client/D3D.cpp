#include "D3D.h"


void CD3D::Init(HWND hwnd, D3DPRESENT_PARAMETERS *d3dpp)
{
	_hwnd = hwnd;
	//**************************时间初始化
	_tApp = GetTickCount(); _tRun = 0; _tLast = _tApp;
	_fps = 60; _msfps = 1000.0f / _fps; _nfps = 0;
	//**************************D3D初始化
	_interface = Direct3DCreate9(D3D_SDK_VERSION);
	if (_interface == NULL) return;

	D3DCAPS9 caps;
	_interface->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &caps);
	int vp = 0;                                    //是否支持硬件顶点运算
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	if (FAILED(_interface->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, vp, d3dpp, &_device)))
	{
		::MessageBox(hwnd, L"CreateDevice Failed", L"Error", MB_OK);
		return;
	}
	//*****************创建文字接口
	if (FAILED(D3DXCreateFont(_device, 22, 10, 10, 0, FALSE, 0, 0, 10, 0, L"FONT", &_font)))
	{
		::MessageBox(hwnd, L"CreateFont Failed", L"Error", MB_OK);
		return;
	}

	D3DXMatrixScaling(&scale1, 1.0f, 1.0f, 1);
	D3DXMatrixScaling(&scale2, 2.2f, 2.2f, 1);

	_moudev.Init(hwnd);
	_keydev.Init(hwnd);

	D3DXCreateSprite(_device, &_sback);
	LoadTextureFR(_device, _tback, MAKEINTRESOURCE(IDB_BACK), 0);
	LoadTextureFR(_device, _ticon, MAKEINTRESOURCE(IDB_ICON), 0);

	//*********************图标显示位置
	vIcon[0] = { 5, 7, 0 };	vIcon[1] = { 56, 7, 0 };	vIcon[2] = { 107, 7, 0 };
	vIcon[3] = { 5, 60, 0 };	vIcon[4] = { 56, 60, 0};	vIcon[5] = { 107, 60, 0 };
	vIcon[6] = { 5, 113, 0 };	vIcon[7] = { 56, 113, 0};	vIcon[8] = { 107, 113, 0 };
	vIcon[9] = { 0, 0, 0};//无用
	vIcon[10] = { 173, 32, 0 };
	vIcon[11] = { 173, 103, 0 };

	rIcon[0] = { 7, 2, 44, 43 };	rIcon[1] = { 63, 2, 108, 48 };	rIcon[2] = { 127, 3, 173, 53 };//黑色，圈叉点
	rIcon[3] = { 7, 84, 43, 126 };	rIcon[4] = { 63, 80, 108, 126 };	rIcon[5] = { 126, 78, 175, 127 };//红色
	rIcon[6] = { 341, 12, 520, 40 };

	rform[0][0] = { 5, 7, 50, 55 }; rform[0][1] = { 56, 7, 101, 55 }; rform[0][2] = { 107, 7, 152, 55 };
	rform[1][0] = { 5, 60, 50, 108 }; rform[1][1] = { 56, 60, 101, 108 }; rform[1][2] = { 107, 60, 152, 108 };
	rform[2][0] = { 5, 113, 50, 161 }; rform[2][1] = { 56, 113, 101, 161 }; rform[2][2] = { 107, 113, 152, 161 };

}

void CD3D::Display()
{
	if (!_device)return;
	_tRun = GetTickCount();

	_device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,128,128), 1.0f, 0);
	//*******************************************场景绘制
	if (SUCCEEDED(_device->BeginScene()))
	{
		_sback->Begin(D3DXSPRITE_ALPHABLEND);

		_sback->SetTransform(&scale1);
		_sback->Draw(_tback, NULL, NULL, NULL, D3DCOLOR_XRGB(255, 255, 255));

		_sback->SetTransform(&scale2);

		if (bIng)//游戏开始
		{
			if (bTurn)//你的回合
			{
				_sback->Draw(_ticon, &rIcon[(bRed ? 5 : 2)], NULL, &vIcon[10], D3DCOLOR_XRGB(255, 255, 255));
				wsprintf(textbuf, L"你的回合(%ds)", oot);
			}
			else//对方的回合
			{
				_sback->Draw(_ticon, &rIcon[(bRed ? 2 : 5)], NULL, &vIcon[11], D3DCOLOR_XRGB(255, 255, 255));
				wsprintf(textbuf, L"对方的回合(%ds)", oot);
			}
			DrawModel(_hwnd, &_pmou, _ticon, &_moudev, _sback);
		}
		else
		{
			if (bReady[0])//用户1准备
			{
				_sback->Draw(_ticon, &rIcon[(bRed ? 5 : 2)], NULL, &vIcon[10], D3DCOLOR_XRGB(255, 255, 255));
				wsprintf(textbuf, L"准备开始...");
			}
			if (bReady[1])//用户2准备
			{
				_sback->Draw(_ticon, &rIcon[(bRed ? 2 : 5)], NULL, &vIcon[11], D3DCOLOR_XRGB(255, 255, 255));
				wsprintf(textbuf, L"准备开始...");
			}
		}
		if (bWin != 0)
		{
			wsprintf(textbuf, L"游戏结束,");
			bTurn = FALSE;
			if (bWin == 1)
				lstrcat(textbuf, L"你赢了！");
			else if (bWin == 2)
				lstrcat(textbuf, L"你输了！");
			else if (bWin == 3)
				lstrcat(textbuf, L"平局了！");

			_sback->Draw(_ticon, &rIcon[(bRed ? 5 : 2)], NULL, &vIcon[10], D3DCOLOR_XRGB(255, 255, 255));
			_sback->Draw(_ticon, &rIcon[(bRed ? 2 : 5)], NULL, &vIcon[11], D3DCOLOR_XRGB(255, 255, 255));
			DrawModel(_hwnd, &_pmou, _ticon, &_moudev, _sback);
		}
		_sback->End();
		_font->DrawText(NULL, textbuf, -1, &rIcon[6], DT_CENTER, D3DCOLOR_XRGB(0, 0, 0));

		_device->EndScene();	
	}
	_device->Present(0, 0, 0, 0);
	
	//****************************同步帧率
	_tLast = GetTickCount();
	Sleep(DWORD( _msfps - (_tRun - _tLast)));
	_tLast = _tRun;
	_tApp += _tLast;
	_nfps++;
	if (_nfps >= _fps)
	{
		_nfps = 0;
		oot = max(0, oot -1);
	}
	if (oot <= 0)bTurn = FALSE;
}

void CD3D::Release()
{
	_moudev.Release();
	_keydev.Release();
	_font->Release();
	_device->Release();
	_interface->Release();
}

void GetMousePosition(POINT *position, HWND hwnd)
{
	GetCursorPos(position);
	ScreenToClient(hwnd, position);
	position->x /= 2.2f;
	position->y /= 2.2f;
}

void DrawModel(HWND hwnd, POINT *pos, LPDIRECT3DTEXTURE9 texture, Mouse *mouse, LPD3DXSPRITE sprite)
{
	GetMousePosition(pos, hwnd);

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
			if (form[i][j] == -1 && bTurn && bWin == 0)//响应用户输入
				if (PtInRect(&rform[i][j], *pos))
				{
					sprite->Draw(texture, &rIcon[(bRed ? 3 : 0) + (bType ? 0 : 1)], NULL, &vIcon[i * 3 + j], DC_T05);
					if (mouse->Hit(0))
					{
						form[i][j] = (bRed ? 3 : 0) + (bType ? 0 : 1);
						bTurn = FALSE;//你的回合结束
						sprite->Draw(texture, &rIcon[(bRed ? 3 : 0) + (bType ? 0 : 1)], NULL, &vIcon[i * 3 + j], DC_T1);
					}
				}
			if (form[i][j] != -1)//绘制棋盘
				sprite->Draw(texture, &rIcon[(int)form[i][j]], NULL, &vIcon[i * 3 + j], DC_T1);
		}
}

