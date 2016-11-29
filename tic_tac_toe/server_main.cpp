#include <winsock.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

void PrintIP(SOCKET temp, int num);
void UpdateClient(SOCKET temp, int num, char form[3][3]);
void UpdateServerFrom(SOCKET temp, int num);

char form[3][3];//����
SOCKET sServer;
SOCKADDR_IN aServer;
SOCKET sClient[2] = { NULL, NULL };
SOCKADDR_IN aClient[2];
int len[2] = { sizeof(aClient[0]), sizeof(aClient[1]) };

BOOL bReady[2];//���û��Ƿ���׼��
BOOL bIng;//��Ϸ�Ƿ�ʼ��
BOOL bFirst;//�Ƿ��ǵ�һ������
BOOL bRed;//�Ƿ��ɫ
BOOL bChess;//��һ���û��Ļغ�
char bWin;//0�������� 1�û�1Ӯ�� 2 Ӯ�� 3ƽ��

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
	std::cout << "[Maxi-��Ϸ������]\n" << std::endl;

	srand(GetTickCount());//�������
	//**********��ʼ����Ϸ״̬
	InitServer();
	//**********��ʼ��Winsock
	WSADATA wsaData;

	aServer.sin_family = AF_INET;
	aServer.sin_port = htons(5150);
	aServer.sin_addr.S_un.S_addr = INADDR_ANY;

	WSAStartup(MAKEWORD(2, 2), &wsaData);

	char name[16];
	hostent *IP;
	gethostname(name, sizeof(name));
	IP = gethostbyname(name);
	std::cout << "����:" << name << std::endl;
	for (int i = 0;; i++)
	{
		std::cout <<"IP" << i << ":" << inet_ntoa(*(IN_ADDR*)IP->h_addr_list[i]) << std::endl;
		if (IP->h_addr_list[i] + IP->h_length >= IP->h_name)break;
	}
	std::cout << std::endl;

	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	
	bind(sServer, (SOCKADDR*)&aServer, sizeof(aServer));

	listen(sServer, 5);

	//************�ȴ��û�1����
	while ((sClient[0] = accept(sServer, (SOCKADDR*)&aClient[0], &len[0])) == NULL);
	PrintIP(sClient[0], 1);
	T0 = CreateThread(NULL, 0, Client0, NULL, 0, 0);

	std::cout << "�ȴ��û�2����" << std::endl;
	//*************�û�2����
	while ((sClient[1] = accept(sServer, (SOCKADDR*)&aClient[1], &len[1])) == NULL);
	PrintIP(sClient[1], 2);
	T1 = CreateThread(NULL, 0, Client1, NULL, 0, 0);

	//************��Ϸ��ѭ��
	while (1)
	{
		while (!(bReady[0] && bReady[1]));
		std::cout << "�û�" << bFirst + 1 << "����" << ":������o:��ɫΪ" << (bFirst ? bRed : !bRed) << std::endl;
		Sleep(3000);
		std::cout << "��Ϸ��ʼ��" << std::endl;
		bChess = bFirst;
		bIng = TRUE;
		while (bWin == 0);
		std::cout << "Ӯ��:" << (int)bWin << std::endl;

		Sleep(3000);//�ȴ�֮ǰ���߳̽���
		//��һ����Ϸ����
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

	std::cout << "�û�" << num << ":[" << inet_ntoa(aName.sin_addr) << "]" << "������" << std::endl;
}

DWORD WINAPI Client0(LPVOID lParam)//��������
{
	char buf[2];
	send(sClient[0], (bRed ? "R" : "B"), 1, 0);
	send(sClient[0], (bFirst ? "o" : "x"), 1, 0);
	CreateThread(NULL, 0, GetReady0, NULL, 0, NULL);
	while (!bReady[1]);	
	send(sClient[0], "Y", 1, 0);
	while (!bIng);//��׼������
	send(sClient[0], "s", 1, 0);
	while (1)//��������
	{
		send(sClient[0], &bWin, 1, 0);//��Ӯ

		if (!bChess)//�غϽ���
		{
			send(sClient[0], "C", 1, 0);
			send(sClient[0], form[0], 3, 0);
			send(sClient[0], form[1], 3, 0);
			send(sClient[0], form[2], 3, 0);
			std::cout << "[�����������û�1����]" << std::endl;
			recv(sClient[0], form[0], 3, 0);
			recv(sClient[0], form[1], 3, 0);
			recv(sClient[0], form[2], 3, 0);
			std::cout << "[�û�1�غϽ���]" << std::endl;
			printf("%2d %2d %2d\n", form[0][0], form[0][1], form[0][2]);
			printf("%2d %2d %2d\n", form[1][0], form[1][1], form[1][2]);
			printf("%2d %2d %2d\n", form[2][0], form[2][1], form[2][2]);

			//�����û�
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
	while (!bReady[0]);	//�û�1δ׼��
	send(sClient[1], "Y", 1, 0);
	while (!bIng);//��׼������
	send(sClient[1], "s", 1, 0);
	while (1)//��������
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
			std::cout << "[�����������û�2����]" << std::endl;
			recv(sClient[1], form[0], 3, 0);
			recv(sClient[1], form[1], 3, 0);
			recv(sClient[1], form[2], 3, 0);
			std::cout << "[�û�2�غϽ���]" << std::endl;
			printf("%2d %2d %2d\n", form[0][0], form[0][1], form[0][2]);
			printf("%2d %2d %2d\n", form[1][0], form[1][1], form[1][2]);
			printf("%2d %2d %2d\n", form[2][0], form[2][1], form[2][2]);
			bChess = !bChess;//�غϽ���
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
		recv(sClient[0], buf, 1, 0);//���ܵ�׼����־
		if (strncmp(buf, "r", 1) == 0)
		{
			std::cout << "�û�1:׼��" << std::endl;
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
		recv(sClient[1], buf, 1, 0);//���ܵ�׼����־
		if (strncmp(buf, "r", 1) == 0)
		{
			std::cout << "�û�2:׼��" << std::endl;
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
	bWin = 0;//δ��ʤ��
	bRed = rand() % 2;
	bFirst = rand() % 2;//ѡ���һ��������û�
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
				continue;//�ж�-1����û��
			}
			else if (i == 0)
			{
				//ÿ��
				if (form[0][j] == form[1][j] && form[1][j] == form[2][j])	bWin = (form[i][j] < 2 ? bRed : !bRed) + 1;
				//б��
				if (j == 0)
					if (form[0][0] == form[1][1] && form[1][1] == form[2][2])	bWin = (form[i][j] < 2 ? bRed : !bRed) + 1;
				if (j == 2)
					if (form[0][2] == form[1][1] && form[1][1] == form[2][0])	bWin = (form[i][j] < 2 ? bRed : !bRed) + 1;
			}
		}
		//ÿ��
		if (form[i][0] == form[i][1] && form[i][1] == form[i][2])
		{
			if (form[i][0] != -1)
				bWin = (form[i][j] < 2 ? bRed : !bRed) + 1;
		}
	}
	if (count == 0 && bWin == 0) bWin = 3;//ƽ��
}