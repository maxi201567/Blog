#include <Windows.h>
#include <ShlObj.h>

#pragma once
class Bitmap
{
public:
	Bitmap() :hBitmap(NULL), hDCBitmap(NULL),hDCPhoto(NULL), mWidth(0), mHeight(0){}
	~Bitmap(){};
	BOOL SaveBitmap()//没带‘/’
	{
		LPSTR   lpImageData = NULL;
		CHAR path[256];
		HANDLE  hFile = NULL;

		::SHGetSpecialFolderPath(0, path, CSIDL_DESKTOPDIRECTORY, 0);

		BITMAP  bm;

		::GetObject(BMout, sizeof(BITMAP), &bm);

		// 设置位图图像数据大小
		DWORD   dwImageSize = bm.bmWidthBytes * bm.bmHeight;

		// 分配内存
		lpImageData = (LPSTR)GlobalAlloc(GPTR, dwImageSize);

		// 设置位图文件头
		BITMAPFILEHEADER bfh;
		bfh.bfType = ((WORD)('M' << 8) | 'B');
		bfh.bfSize = sizeof(BITMAPFILEHEADER)+
			sizeof(BITMAPINFOHEADER)+
			dwImageSize;
		bfh.bfReserved1 = 0;
		bfh.bfReserved2 = 0;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER)+
			sizeof(BITMAPINFOHEADER);

		// 设置位图信息头
		BITMAPINFOHEADER bih;
		bih.biSize = sizeof(BITMAPINFOHEADER); // 位图信息头尺寸
		bih.biWidth = bm.bmWidth;     // 位图宽度
		bih.biHeight = bm.bmHeight;    // 位图高度
		bih.biBitCount = bm.bmBitsPixel; // 设置每像素所用字节数
		bih.biCompression = 0;
		bih.biPlanes = 1;
		bih.biSizeImage = dwImageSize;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrImportant = 0;
		bih.biClrUsed = 0;
		// 获取位图图像数据
		GetDIBits(hDCPhoto, BMout, 0, bih.biHeight, lpImageData, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

		// 设置文件名
		SYSTEMTIME st;
		GetSystemTime(&st);
		TCHAR szFileName[_MAX_PATH] = "\0";
		wsprintf(szFileName, TEXT("%d%d%d%d%d.bmp"),
			st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond);


		TCHAR szAbsolutePath[_MAX_PATH] = "\0";
		wsprintf(szAbsolutePath, TEXT("%s\\%s"), path, szFileName);

		// 创建文件，存储位图
		hFile = CreateFile(szAbsolutePath,           // open file 
			GENERIC_WRITE,            // open for writing 
			0,                        // do not share 
			NULL,                     // no security 
			OPEN_ALWAYS,              // Opens the file, if it exists. If the file does not exist, the function creates the file 
			FILE_ATTRIBUTE_NORMAL |   // normal file 
			FILE_FLAG_WRITE_THROUGH,  // Instructs the system to write through any intermediate cache and go directly to disk. 
			NULL);                    // no attr. template 


		DWORD dwWriteBytes = 0;
		::WriteFile(hFile, &bfh, sizeof(BITMAPFILEHEADER), &dwWriteBytes, NULL);
		::WriteFile(hFile, &bih, sizeof(BITMAPINFOHEADER), &dwWriteBytes, NULL);
		::WriteFile(hFile, lpImageData, dwImageSize, &dwWriteBytes, NULL);

		// 关闭文件
		CloseHandle(hFile);

		// 释放内存
		GlobalFree(lpImageData);

		return TRUE;

	}
	BOOL ScreenShot()
	{

		if (hDCBitmap == NULL)
		{
			hDCBitmap = ::CreateDC("DISPLAY", NULL, NULL, NULL);
			hDCMem = ::CreateCompatibleDC(hDCBitmap);
			hDCTemp = ::CreateCompatibleDC(hDCBitmap);

			mWidth = GetSystemMetrics(SM_CXSCREEN); // 屏幕宽度
			mHeight = GetSystemMetrics(SM_CYSCREEN); // 屏幕高度

			// 创建位图
			hBitmap = ::CreateCompatibleBitmap(hDCBitmap, mWidth, mHeight);
			hBmTemp = ::CreateCompatibleBitmap(hDCBitmap, mWidth, mHeight);
			::SelectObject(hDCMem, hBitmap);
			::SelectObject(hDCTemp, hBmTemp);

			::BitBlt(hDCMem, 0, 0, mWidth, mHeight, hDCBitmap, 0, 0, SRCCOPY);
			::BitBlt(hDCTemp, 0, 0, mWidth, mHeight, hDCBitmap, 0, 0, SRCCOPY);
		}
		return TRUE;

	}
	HDC Bitmap::GetBackDC()
	{
		return hDCMem;
	}
	void CleanUp()
	{
		::DeleteObject(hBitmap);
		::DeleteObject(hBmTemp);
		::DeleteObject(BMout);
		::DeleteDC(hDCBitmap);
		::DeleteDC(hDCMem);
		::DeleteDC(hDCTemp);
		::DeleteDC(hDCPhoto);
		hBitmap = hBmTemp = BMout = NULL;
		hDCBitmap = hDCMem = hDCPhoto = hDCTemp = NULL;
	}
	void DrawRect(RECT* rtemp)
	{
		::BitBlt(hDCMem, 0, 0, mWidth, mHeight, hDCTemp, 0, 0, SRCCOPY);//恢复未按的状态
		memcpy(&rect, rtemp, sizeof(RECT));
		HBRUSH brush = (HBRUSH)GetStockObject(NULL_BRUSH);
		HPEN pen = CreatePen(PS_DASH, 1, RGB(255,0,255));
		SelectObject(hDCMem, brush);
		SelectObject(hDCMem, pen);
		Rectangle(hDCMem, rect.left, rect.top, rect.right, rect.bottom);
	}
	HDC GetPhotoDC()
	{
		if (hDCPhoto == NULL)
			hDCPhoto = ::CreateCompatibleDC(hDCTemp);
		if (rect.bottom == rect.top || rect.left == rect.right)
			return NULL;
		int temp;
		if (rect.left > rect.right)
		{
			temp = rect.left;
			rect.left = rect.right;
			rect.right = temp;
		}
		if (rect.top > rect.bottom)
		{
			temp = rect.top;
			rect.top = rect.bottom;
			rect.bottom = temp;
		}
		BMout = ::CreateCompatibleBitmap(hDCTemp, rect.right - rect.left, rect.bottom - rect.top);
		::SelectObject(hDCPhoto, BMout);
		::StretchBlt(hDCPhoto, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
			hDCTemp, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SRCCOPY);

		return hDCPhoto;
	}
	int GetWidth(){ return rect.right - rect.left; }
	int GetHeight(){ return rect.bottom - rect.top; }
private:
	RECT rect;
	HBITMAP BMout;
	HBITMAP hBitmap, hBmTemp;//hBitmap获取最开始的截图，hBmTemp保存这张截图用于恢复
	HDC   hDCBitmap;//用于创建hBitmap
	HDC   hDCMem, hDCTemp, hDCPhoto;//hDCMem后台缓存，保存绘制数据，存放hBitmap
	                                //hDCTemp存放hBmTemp，用于恢复原始截图，恢复到hDCMem
	                                //hDCPhoto存放BMout，截图截取的区域的截图
	int mWidth;
	int mHeight;
};

