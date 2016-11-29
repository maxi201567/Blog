#include <Windows.h>

#pragma once
class CWFrame
{
private:
	HWND hwnd;
	MSG msg;

public:

	CWFrame() :hwnd(NULL){};
	~CWFrame();
	void Init(HINSTANCE hInstance, int nShowCmd, int Width, int Height, bool bWindow);//创建一个主窗口
	int MsgLoop();
	void Release();

};

//标记
class CPEN
{
private:
	struct _point
	{
		int X, Y;
		BOOL breakPoint;
		COLORREF color;
		int size;
		_point *next;
	};
	_point* header;
	_point* current;
	int count;
public:

	CPEN() :header(NULL), count(0)
	{
		current = header;
	}

	int Insert(int x, int y, BOOL bkP, COLORREF c,int s)//返回插入的第几个点
	{
		_point *nextpoint = new _point{ x, y, bkP, c, s, NULL };
		count++;
		current = header;
		if (header == NULL)
		{
			header = nextpoint;
			return count;
		}
		while (current->next != NULL)current = current->next;
		current->next = nextpoint;

		return count;
	}
	void Display(HDC hdc)
	{
		_point* temp;
		HPEN pen;
		current = header;
		temp = current;

		if (current == NULL) return;

		pen = CreatePen(0, current->size, current->color);
		SelectObject(hdc, pen);

		while (current != NULL)//为NULL时证明已经画完
		{
			if (temp->color != current->color || temp->size != current->size)
			{
				pen = CreatePen(0, current->size, current->color);
				SelectObject(hdc, pen);
			}
			if (current->next != NULL && !current->breakPoint)
			{
				MoveToEx(hdc, current->X, current->Y, NULL);
				LineTo(hdc, current->next->X, current->next->Y);
			}
			temp = current;
			current = current->next;
		}
	}
	void Empty()
	{
		_point* temp;
		current = header;
		temp = header;
		while (current != NULL)
		{
			current = current->next;
			delete temp;
			temp = current;
		}
		header = NULL;
		current = NULL;
		count = 0;
	}
};
const COLORREF COLOR[3] = { RGB(255, 0, 0), RGB(0, 255, 0), RGB(0, 0, 255) };
const int DEGREE[3] = { 3, 6, 10 };


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);//消息循环
LRESULT CALLBACK ShotProc(HWND hShot, UINT msg, WPARAM wParam, LPARAM lParam);//消息循环
LRESULT CALLBACK NoteProc(HWND hNote, UINT msg, WPARAM wParam, LPARAM lParam);//消息循环
