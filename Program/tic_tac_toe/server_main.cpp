#include <winsock.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

void PrintIP(SOCKET temp, int num);
void UpdateClient(SOCKET temp, int num, char form[3][3]);
void UpdateServerFrom(SOCKET temp, int num);

char form[3][3];//棋阵
SOCKET sServer;
SOCKADDR_IN aServer;
SOCKET sClient[2] = { NULL, NULL };
SOCKADDR_IN aClient[2];
int len[2] = { sizeof(aClient[0]), sizeof(aClient[1]) };

BOOL bReady[2];//各用户是否已准备
BOOL bIng;//游戏是否开始了
BOOL bFirst;//是否是第一个下棋
BOOL bRed;//是否红色
BOOL bChess;//第一个用户的回合
char bWin;//0正常下棋 1用户1赢了 2 赢了 3平局

//bRed: R:B  bReady r Y  bFirst o:x
//Turn t  Start s

DWORD WINAPI Client0(LPVOID lParam);
DWORD WINAPI Client1(LPVOID lParam);
DWORD WINAPI GetReady0(LPVOID lParam);
DWORD WINAPI GetReady1(LPVOID lParam);

void InitServer();
void CheckWiner();

HANDLE T0, T1;

int main()
{
	std::cout << "[Maxi-游戏服务器]\n" << std::endl;

	srand(GetTickCount());//随机种子
	//**********初始化游戏状态
	InitServer();
	//**********初始化Winsock
	WSADATA wsaData;

	aServer.sin_family = AF_INET;
	aServer.sin_port = htons(5150);
	aServer.sin_addr.S_un.S_addr = INADDR_ANY;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	char name[16];
	hostent *IP;
	gethostname(name, sizeof(name));
	IP = gethostbyname(name);
	std::cout << "本机:" << name << std::endl;
	for (int i = 0;; i++)
	{
		std::cout <<"IP" << i << ":" << inet_ntoa(*(IN_ADDR*)IP->h_addr_list[i]) << std::endl;
		if (IP->h_addr_list[i] + IP->h_length >= IP->h_name)break;
	}
	std::cout << std::endl;

	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	
	bind(sServer, (SOCKADDR*)&aServer, sizeof(aServer));

	listen(sServer, 5);

	//************等待用户1连接
	while ((sClient[0] = accept(sServer, (SOCKADDR*)&aClient[0], &len[0])) == NULL);
	PrintIP(sClient[0], 1);
	T0 = CreateThread(NULL, 0, Client0, NULL, 0, 0);

	std::cout << "等待用户2连接" << std::endl;
	//*************用户2连接
	while ((sClient[1] = accept(sServer, (SOCKADDR*)&aClient[1], &len[1])) == NULL);
	PrintIP(sClient[1], 2);
	T1 = CreateThread(NULL, 0, Client1, NULL, 0, 0);

	//************游戏主循环
	while (1)
	{
		while (!(bReady[0] && bReady[1]));
		std::cout << "用户" << bFirst + 1 << "先手" << ":类型是o:颜色为" << (bFirst ? bRed : !bRed) << std::endl;
		Sleep(3000);
		std::cout << "游戏开始了" << std::endl;
		bChess = bFirst;
		bIng = TRUE;
		while (bWin == 0);
		std::cout << "赢家:" << (int)bWin << std::endl;

		Sleep(3000);//等待之前的线程结束
		//上一场游戏结束
		InitServer();
		TerminateThread(T0, 0);
		TerminateThread(T1, 0);
		T0 = CreateThread(NULL, 0, Client0, NULL, 0, 0);
		T1 = CreateThread(NULL, 0, Client1, NULL, 0, 0);
	}

	while (1);

	closesocket(sServer);
	closesocket(sClient[0]);
	closesocket(sClient[1]);
	WSACleanup();

	return 0;
}

void PrintIP(SOCKET temp, int num)
{
	SOCKADDR_IN aName;
	int len = sizeof(aName);

	getpeername(temp, (SOCKADDR*)&aName, &len);

	std::cout << "用户" << num << ":[" << inet_ntoa(aName.sin_addr) << "]" << "已连接" << std::endl;
}

DWORD WINAPI Client0(LPVOID lParam)//更新数据
{
	char buf[2];
	send(sClient[0], (bRed ? "R" : "B"), 1, 0);
	send(sClient[0], (bFirst ? "o" : "x"), 1, 0);
	CreateThread(NULL, 0, GetReady0, NULL, 0, NULL);
	while (!bReady[1]);	
	send(sClient[0], "Y", 1, 0);
	while (!bIng);//都准备好了
	send(sClient[0], "s", 1, 0);
	while (1)//接受棋盘
	{
		send(sClient[0], &bWin, 1, 0);//输赢

		if (!bChess)//回合结束
		{
			send(sClient[0], "C", 1, 0);
			send(sClient[0], form[0], 3, 0);
			send(sClient[0], form[1], 3, 0);
			send(sClient[0], form[2], 3, 0);
			std::cout << "[数据已往用用户1发送]" << std::endl;
			recv(sClient[0], form[0], 3, 0);
			recv(sClient[0], form[1], 3, 0);
			recv(sClient[0], form[2], 3, 0);
			std::cout << "[用户1回合结束]" << std::endl;
			printf("%2d %2d %2d\n", form[0][0], form[0][1], form[0][2]);
			printf("%2d %2d %2d\n", form[1][0], form[1][1], form[1][2]);
			printf("%2d %2d %2d\n", form[2][0], form[2][1], form[2][2]);

			//更换用户
			bChess = !bChess;
		}
		else
		{
			send(sClient[0], "N", 1, 0);
			while (bChess);
		}
		if (bWin != 0)break;
		CheckWiner();
	}

	return 0;
}
DWORD WINAPI Client1(LPVOID lParam)
{
	char buf[2];
	send(sClient[1], (bRed ? "B" : "R"), 1, 0);
	send(sClient[1], (bFirst ? "x" : "o"), 1, 0);
	CreateThread(NULL, 0, GetReady1, NULL, 0, NULL);
	while (!bReady[0]);	//用户1未准备
	send(sClient[1], "Y", 1, 0);
	while (!bIng);//都准备好了
	send(sClient[1], "s", 1, 0);
	while (1)//接受棋盘
	{
		if (bWin == 2)
		{
			buf[0] = 1;
			send(sClient[1], &buf[0], 1, 0);
		}
		else if (bWin == 1)
		{
			buf[0] = 2;
			send(sClient[1], &buf[0], 1, 0);
		}
		else send(sClient[1], &bWin, 1, 0);

		if (bChess)
		{
			send(sClient[1], "C", 1, 0);
			send(sClient[1], form[0], 3, 0);
			send(sClient[1], form[1], 3, 0);
			send(sClient[1], form[2], 3, 0);
			std::cout << "[数据已往用用户2发送]" << std::endl;
			recv(sClient[1], form[0], 3, 0);
			recv(sClient[1], form[1], 3, 0);
			recv(sClient[1], form[2], 3, 0);
			std::cout << "[用户2回合结束]" << std::endl;
			printf("%2d %2d %2d\n", form[0][0], form[0][1], form[0][2]);
			printf("%2d %2d %2d\n", form[1][0], form[1][1], form[1][2]);
			printf("%2d %2d %2d\n", form[2][0], form[2][1], form[2][2]);
			bChess = !bChess;//回合结束
		}
		else
		{
			send(sClient[1], "N", 1, 0);
			while (!bChess);
		}
		if (bWin != 0)break;
		CheckWiner();
	}

	return 0;
}

DWORD WINAPI GetReady0(LPVOID lParam)
{
	char buf[2];
	while (1)
	{
		recv(sClient[0], buf, 1, 0);//接受到准备标志
		if (strncmp(buf, "r", 1) == 0)
		{
			std::cout << "用户1:准备" << std::endl;
			bReady[0] = TRUE;
			break;
		}
	}
	return 0;
}
DWORD WINAPI GetReady1(LPVOID lParam)
{
	char buf[2];
	while (1)
	{
		recv(sClient[1], buf, 1, 0);//接受到准备标志
		if (strncmp(buf, "r", 1) == 0)
		{
			std::cout << "用户2:准备" << std::endl;
			bReady[1] = TRUE;
			break;
		}
	}
	return 0;
}

void InitServer()
{
	memset(form, -1, sizeof(form));//0 1 3 4
	bReady[0] = bReady[1] = FALSE;
	bWin = 0;//未分胜负
	bRed = rand() % 2;
	bFirst = rand() % 2;//选择第一个下棋的用户
	bIng = FALSE;
}
void CheckWiner()
{
	if (bWin != 0)return;
	int i, j, count = 0;
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (form[i][j] == -1)
			{
				count++;
				continue;//判断-1还有没有
			}
			else if (i == 0)
			{
				//每列
				if (form[0][j] == form[1][j] && form[1][j] == form[2][j])	bWin = (form[i][j] < 2 ? bRed : !bRed) + 1;
				//斜线
				if (j == 0)
					if (form[0][0] == form[1][1] && form[1][1] == form[2][2])	bWin = (form[i][j] < 2 ? bRed : !bRed) + 1;
				if (j == 2)
					if (form[0][2] == form[1][1] && form[1][1] == form[2][0])	bWin = (form[i][j] < 2 ? bRed : !bRed) + 1;
			}
		}
		//每行
		if (form[i][0] == form[i][1] && form[i][1] == form[i][2])
		{
			if (form[i][0] != -1)
				bWin = (form[i][j] < 2 ? bRed : !bRed) + 1;
		}
	}
	if (count == 0 && bWin == 0) bWin = 3;//平局
}