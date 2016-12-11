#define _AFXDLL
#include <afx.h>
#include <ShlObj.h>
#include <Windows.h>
#include <Windowsx.h>
#include <commdlg.h>
#include <winsock.h>
#include <string>
#include <fstream>
#include "resource.h"
#pragma comment(lib, "Ws2_32.lib")

#include "filestruct.h"

SOCKET localS;
SOCKET farS;
SOCKADDR_IN localA;
SOCKADDR_IN farA;
//*******************************
//MaxiChen-2016.12.4-Begin
//假设每台电脑数据缓冲区都大于256
//*******************************

void Recv(HWND hDialog);
void Send(HWND hDialog);//文件发送
void SendData(HWND hDialog, char *content, int size);
void Print(HWND hDialog, bool bsend);
void GetFileInfo(HWND hDialog);


BOOL CALLBACK DialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmd, int nShowCmd)
{
	WSAData data;
	WSAStartup(MAKEWORD(2, 2), &data);
	farA.sin_family = PF_INET;
	farA.sin_port = htons(5150);
	
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG), NULL, DialogProc);
	WSACleanup();
	return 1;
}

data_type nextType = data_type::Div;

char name[16];
hostent *IP;
char text[256];
char sdrvbuf[512];
char *filebuf;
int len = sizeof(SOCKADDR_IN);
CString content;
OPENFILENAME file;
BROWSEINFO   bf;
LPITEMIDLIST pidl;
TCHAR path[256];
TCHAR filename[256];
int filesize = 0;
CString FileName;//const
bool bRecvFile = false;
bool bSendFile = false;

int line = 0;

BOOL _stdcall DialogProc(HWND hDialog, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	RECT rect;
	switch (message)
	{
	case WM_INITDIALOG:
		gethostname(name, sizeof(name));
		IP = gethostbyname(name);
		hwnd = GetDlgItem(hDialog, IDC_HIP);
		for (int i = 0;; i++)
		{
			CString p = inet_ntoa(*(IN_ADDR*)IP->h_addr_list[i]);
			ComboBox_AddString(hwnd, p.GetString());
			if (IP->h_addr_list[i] + IP->h_length >= IP->h_name)
			{    
				ComboBox_SetCurSel(hwnd, 0);
				GetWindowRect(hwnd, &rect);
				POINT temp = { rect.left, rect.top };
				ScreenToClient(hDialog, &temp);
				MoveWindow(hwnd, temp.x, temp.y, rect.right - rect.left, (i + 2)*(rect.bottom - rect.top), true);
				break;
			}
		}
		localS = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		farS = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		localA.sin_family = farA.sin_family = AF_INET;
		localA.sin_port = farA.sin_port = htons(5150);
		localA.sin_addr.S_un.S_addr = farA.sin_addr.S_un.S_addr = INADDR_ANY;

	//	hwnd = GetDlgItem(hDialog, IDC_SEND);
	//	SetWindowFont(hwnd, GetStockFont(0), true);
		return true;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BIND:
			GetWindowTextA(GetDlgItem(hDialog, IDC_HPORT), text, 256);
			localA.sin_port = htons(atoi(text));
			GetWindowTextA(GetDlgItem(hDialog, IDC_FPORT), text, 256);
			farA.sin_port = htons(atoi(text));
			if (bind(localS, (SOCKADDR*)&localA, sizeof(localA)) == SOCKET_ERROR)
			{
				char temp[64];
				sprintf(temp, "BIND ERROR\nERROR CODE:%d", WSAGetLastError());
				SetWindowTextA(GetDlgItem(hDialog, IDC_E_SEND), temp);
			}
			else
			{
				char temp[64];
				sprintf(temp, "Bind Succeed");
				SetWindowTextA(GetDlgItem(hDialog, IDC_E_SEND), temp);
			}
			WSAAsyncSelect(localS, hDialog, IDM_ASYNC, FD_READ | FD_WRITE | FD_CLOSE);
			break;

		case IDC_OPEN_FILE:
			ZeroMemory(&file, sizeof(file));
			ZeroMemory(&filename, sizeof(filename));
			file.lStructSize = sizeof(file);
			file.Flags = OFN_HIDEREADONLY;
			file.hwndOwner = hDialog;
			file.lpstrFile = filename;
			file.nMaxFile = 256;
			file.lpstrFilter = L"All Files(*.*)\0*.*\0";
			if (GetOpenFileName(&file))
			{
				hwnd = GetDlgItem(hDialog, IDC_SEND);
				CString temp = "[File]-[";
				temp += file.lpstrFile;
				temp += "]";
				SetWindowText(hwnd, temp.GetString());
				//读入文件数据
				FileName = file.lpstrFile;
				HANDLE infile = CreateFile(FileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (infile != INVALID_HANDLE_VALUE)
				{
					hwnd = GetDlgItem(hDialog, IDC_E_SEND);
					char show[64];
					sprintf(show, "Open file Succeed.\nSize:%d Bytes", GetFileSize(infile, NULL));
					SetWindowTextA(hwnd, show);
					hwnd = GetDlgItem(hDialog, IDC_SEND);
					sprintf(show, "-[%d Bytes]", GetFileSize(infile, NULL));
					temp += show;
					SetWindowText(hwnd, temp.GetString());
				}
				else
				{
					hwnd = GetDlgItem(hDialog, IDC_E_SEND);
					char show[64];
					sprintf(show, "Open file Failed");
					SetWindowTextA(hwnd, show);
				}
				CloseHandle(infile);
			}
			break;
			
		case IDC_SAVE_FILE:

			bf.hwndOwner = hDialog;
			bf.lpszTitle = L"选择路径";
			bf.ulFlags = BIF_RETURNONLYFSDIRS;     
			pidl = SHBrowseForFolder(&bf);
			if (SHGetPathFromIDList(pidl, path))
			{
				int a = lstrlen(path);
				if (path[a - 1] != '\\')
				{
					path[a + 1] = '\0';
					path[a] = '\\';
				}
				GetWindowTextA(GetDlgItem(hDialog, IDC_FIP), text, 256);
				farA.sin_addr.S_un.S_addr = inet_addr(text);

				EnableWindow(GetDlgItem(hDialog, IDC_SAVE_FILE), false);

				if (line > 15)
				{
					line = 0;
					content.Empty();
				}
				SYSTEMTIME time;
				GetLocalTime(&time);
				sprintf(text, "[%d:%d]- | |  ", time.wHour, time.wMinute);

				content += text;
				content += "[File]Save as:";
				content += path;
				content += filename;
				content += '\n';
				SetWindowText(GetDlgItem(hDialog, IDC_CONTENT), content);

				FileName.Empty();
				FileName = path;
				FileName += filename;

				SendData(hDialog, "[FILE]", 6);//准备接收文件
				SendData(hDialog, "start", 5);//占位
				bRecvFile = true;
				bSendFile = false;
			}
			break;

			//发送消息
		case IDB_SEND:

			ZeroMemory(sdrvbuf, sizeof(sdrvbuf));
			GetWindowTextA(GetDlgItem(hDialog, IDC_FIP), text, 256);
			farA.sin_addr.S_un.S_addr = inet_addr(text);

			GetWindowTextA(GetDlgItem(hDialog, IDC_SEND), sdrvbuf, 512);
			SetWindowTextA(GetDlgItem(hDialog, IDC_SEND), "");
			//发送数据
			if (strncmp("[File]", sdrvbuf, 6) == 0)//发送文件
			{
				Print(hDialog, true);
				CStringA t = sdrvbuf;
				t.Delete(8, t.ReverseFind('\\') - 7);
				ZeroMemory(sdrvbuf, sizeof(sdrvbuf));
				strcpy(sdrvbuf, t);
				EnableWindow(GetDlgItem(hDialog, IDC_SAVE_FILE), false);
				SendData(hDialog, "[CLER]", 6);
				SendData(hDialog, "[FILE]", 6);//告诉对方要发送文件了
				SendData(hDialog, sdrvbuf, strlen(sdrvbuf));//发送文件信息
				//设置为文件发送端
				bSendFile = true;
				bRecvFile = false;
			}
			else//发送普通文字信息
			{
				SendData(hDialog, "[TEXT]", 6);
				SendData(hDialog, sdrvbuf, strlen(sdrvbuf));
				Print(hDialog, true);
			}
			break;
		case IDCANCEL:
			EndDialog(hDialog, 0);
			break;
		}
		return true;

	case IDM_ASYNC:
		if (WSAGETSELECTERROR(lParam))
		{
			MessageBoxA(NULL, "!!!", "ERROR", MB_OK | MB_ICONERROR);
		}
		switch (WSAGETSELECTEVENT(lParam))
		{
		case FD_READ:
			Recv(hDialog);			
			break;
		case FD_CLOSE:
			break;
		}
		return true;
	}
	return false;
}

HANDLE infile = INVALID_HANDLE_VALUE;
DWORD filehadread = 0;
DWORD filehadwrite = 0;
DWORD hadsend = 0;

void Recv(HWND hDialog)
{
	HWND hwnd;
	ZeroMemory(sdrvbuf, sizeof(sdrvbuf));
	int recvcount = recvfrom(localS, sdrvbuf, 512, 0, (SOCKADDR*)&farA, &len);
	if (recvcount == SOCKET_ERROR)
	{
		char temp[64];
		sprintf(temp, "ERROR CODE:%d.", WSAGetLastError());
		SetWindowTextA(GetDlgItem(hDialog, IDC_E_RECV), temp);
	}
	else//接收数据成功
	{
		if (nextType == data_type::Div)
		{
			//普通文字信息
			if (strncmp(sdrvbuf, "[TEXT]", 6) == 0)
				nextType = data_type::Text;
			else if (strncmp(sdrvbuf, "[FILE]", 6) == 0)//提示有文件要接收
			{
				//发送端接收到[FILE]
				//发送端，开始发送文件
				if (bSendFile)
				{
					ZeroMemory(sdrvbuf, sizeof(sdrvbuf));
					sprintf(sdrvbuf, "[File]Your file had been receive.");
					Print(hDialog, false);
					nextType = data_type::FileCTNU;
				}
				else
				{
					//接收端接收到[FILE]
					//接收端，读取文件名和大小
					nextType = data_type::FileBGIN;
				}
			}
			else if (strncmp(sdrvbuf, "[CTNU]", 6) == 0)
				nextType = data_type::FileCTNU;
			else if (strncmp(sdrvbuf, "[CLER]", 6) == 0)
			{
				filehadread = 0;
				filehadwrite = 0;
				hadsend = 0;
				bSendFile = false;
				bRecvFile = false;
				nextType = data_type::Div;
			}
			else if (strncmp(sdrvbuf, "[DONE]", 6) == 0)
			{
				ZeroMemory(sdrvbuf, sizeof(sdrvbuf));
				sprintf(sdrvbuf, "[File]Finished");
				Print(hDialog, false);
				filehadread = 0;
				filehadwrite = 0;
				hadsend = 0;
				bSendFile = false;
				bRecvFile = false;
				nextType = data_type::Div;
			}

		}
		else if (nextType == data_type::Text)
		{
			Print(hDialog, false);
			nextType = data_type::Div;
		}
		else if (nextType == data_type::FileBGIN)
		{
			recvfrom(localS, sdrvbuf, 512, 0, (SOCKADDR*)&farA, &len);
			Print(hDialog, false);
			GetFileInfo(hDialog);
			EnableWindow(GetDlgItem(hDialog, IDC_SAVE_FILE), true);
			nextType = data_type::Div;
		}
		else if (nextType == data_type::FileCTNU)
		{
			//发送端收到continue直接发送数据
			if (bSendFile)
			{
				infile = CreateFile(FileName, GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (infile == INVALID_HANDLE_VALUE)
				{
					SetWindowTextA(GetDlgItem(hDialog, IDC_E_SEND), "INVALID_HANDLE_VALUE");
					CloseHandle(infile);
					return;
				}
				SetFilePointer(infile, hadsend, 0, FILE_BEGIN);
				if (!ReadFile(infile, (void*)sdrvbuf, 256, &filehadread, NULL))
					SetWindowTextA(GetDlgItem(hDialog, IDC_E_SEND), "ReadFile failed!");
				hadsend += filehadread;
				CloseHandle(infile);

				SendData(hDialog, "[CTNU]", 6);
				SendData(hDialog, sdrvbuf, filehadread);
			}
			if (bRecvFile)
			{
				infile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (infile == INVALID_HANDLE_VALUE)
				{
					ZeroMemory(text, sizeof(text));
					sprintf(text, "INVALID_HANDLE_VALUE\nError code:%d",GetLastError());

					SetWindowTextA(GetDlgItem(hDialog, IDC_E_RECV), text);
					CloseHandle(infile);
					return;
				}
				SetFilePointer(infile, 0, 0, FILE_END);
				DWORD size = min(256, filesize - GetFileSize(infile, NULL));

				if (!WriteFile(infile, (void*)sdrvbuf, size, &filehadwrite, NULL))
					SetWindowTextA(GetDlgItem(hDialog, IDC_E_RECV), "WriteFile failed!");

				if (GetFileSize(infile, NULL) >= filesize)//接收完成
				{
					CloseHandle(infile);
					SendData(hDialog, "[DONE]", 6);
					ZeroMemory(sdrvbuf, sizeof(sdrvbuf));
					sprintf(sdrvbuf, "[File] Finished!");
					Print(hDialog, false);
					bRecvFile = false;
					bSendFile = false;
				}
				else
				{
					CloseHandle(infile);
					SendData(hDialog, "[CTNU]", 6);
					SendData(hDialog, "start", 5);//占位
				}
			}
			nextType = data_type::Div;
		}
	}
}
void SendData(HWND hDialog, char *content, int size)
{
	if (sendto(localS, content, size, 0, (SOCKADDR*)&farA, sizeof(farA)) == SOCKET_ERROR)
	{
		sprintf(text, "Error:\nSend data failed\nError Code:%d", WSAGetLastError());
		SetWindowTextA(GetDlgItem(hDialog, IDC_E_SEND), text);
	}
	else
	{
		sprintf(text, "Send data succeed!");
		SetWindowTextA(GetDlgItem(hDialog, IDC_E_SEND), text);
	}
}

void Print(HWND hDialog, bool bsend)
{
	if (line > 15)
	{
		line = 0;
		content.Empty();
	}
	SYSTEMTIME time;
	GetLocalTime(&time);
	if (bsend)
		sprintf(text, "[%d:%d]-<<  ", time.wHour, time.wMinute);
	else
		sprintf(text, "[%d:%d]->>  ", time.wHour, time.wMinute);

	content += text;
	content += sdrvbuf;
	content += "\n";
	SetWindowText(GetDlgItem(hDialog, IDC_CONTENT), content.GetString());
	line++;
}
void GetFileInfo(HWND hDialog)
{
	//还没开始接收文件时，找出文件名称和文件大小
	//即第一条文件消息
	CString name = sdrvbuf;
	name.Delete(0, 8);
	int a = name.Find(']');
	name.Delete(a, name.GetLength() - a);
	lstrcpy(filename, name.GetString());//获取文件名
	name = sdrvbuf;
	name.Delete(0, 8);
	a = name.Find('[');
	name.Delete(0, a + 1);
	name.Delete(name.GetLength() - 7, 7);
	filesize = _ttoi(name.GetString());
}	  