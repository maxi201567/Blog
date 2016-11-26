#include <Windows.h>
#include <ShlObj.h>

#pragma once
class Bitmap
{
public:
	Bitmap() :hBitmap(NULL), hDCBitmap(NULL),hDCPhoto(NULL), mWidth(0), mHeight(0){}
	~Bitmap(){};
	BOOL SaveBitmap()//û����/��
	{
		LPSTR   lpImageData = NULL;
		CHAR path[256];
		HANDLE  hFile = NULL;

		::SHGetSpecialFolderPath(0, path, CSIDL_DESKTOPDIRECTORY, 0);

		BITMAP  bm;

		::GetObject(BMout, sizeof(BITMAP), &bm);

		// ����λͼͼ�����ݴ�С
		DWORD   dwImageSize = bm.bmWidthBytes * bm.bmHeight;

		// �����ڴ�
		lpImageData = (LPSTR)GlobalAlloc(GPTR, dwImageSize);

		// ����λͼ�ļ�ͷ
		BITMAPFILEHEADER bfh;
		bfh.bfType = ((WORD)('M' << 8) | 'B');
		bfh.bfSize = sizeof(BITMAPFILEHEADER)+
			sizeof(BITMAPINFOHEADER)+
			dwImageSize;
		bfh.bfReserved1 = 0;
		bfh.bfReserved2 = 0;
		bfh.bfOffBits = sizeof(BITMAPFILEHEADER)+
			sizeof(BITMAPINFOHEADER);

		// ����λͼ��Ϣͷ
		BITMAPINFOHEADER bih;
		bih.biSize = sizeof(BITMAPINFOHEADER); // λͼ��Ϣͷ�ߴ�
		bih.biWidth = bm.bmWidth;     // λͼ���
		bih.biHeight = bm.bmHeight;    // λͼ�߶�
		bih.biBitCount = bm.bmBitsPixel; // ����ÿ���������ֽ���
		bih.biCompression = 0;
		bih.biPlanes = 1;
		bih.biSizeImage = dwImageSize;
		bih.biXPelsPerMeter = 0;
		bih.biYPelsPerMeter = 0;
		bih.biClrImportant = 0;
		bih.biClrUsed = 0;
		// ��ȡλͼͼ������
		GetDIBits(hDCPhoto, BMout, 0, bih.biHeight, lpImageData, (BITMAPINFO*)&bih, DIB_RGB_COLORS);

		// �����ļ���
		SYSTEMTIME st;
		GetSystemTime(&st);
		TCHAR szFileName[_MAX_PATH] = "\0";
		wsprintf(szFileName, TEXT("%d%d%d%d%d.bmp"),
			st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond);


		TCHAR szAbsolutePath[_MAX_PATH] = "\0";
		wsprintf(szAbsolutePath, TEXT("%s\\%s"), path, szFileName);

		// �����ļ����洢λͼ
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

		// �ر��ļ�
		CloseHandle(hFile);

		// �ͷ��ڴ�
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

			mWidth = GetSystemMetrics(SM_CXSCREEN); // ��Ļ���
			mHeight = GetSystemMetrics(SM_CYSCREEN); // ��Ļ�߶�

			// ����λͼ
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
		::BitBlt(hDCMem, 0, 0, mWidth, mHeight, hDCTemp, 0, 0, SRCCOPY);//�ָ�δ����״̬
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
	HBITMAP hBitmap, hBmTemp;//hBitmap��ȡ�ʼ�Ľ�ͼ��hBmTemp�������Ž�ͼ���ڻָ�
	HDC   hDCBitmap;//���ڴ���hBitmap
	HDC   hDCMem, hDCTemp, hDCPhoto;//hDCMem��̨���棬����������ݣ����hBitmap
	                                //hDCTemp���hBmTemp�����ڻָ�ԭʼ��ͼ���ָ���hDCMem
	                                //hDCPhoto���BMout����ͼ��ȡ������Ľ�ͼ
	int mWidth;
	int mHeight;
};

