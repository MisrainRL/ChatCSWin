#include <winsock2.h>
#include <ws2tcpip.h>
#include "framework.h"
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#define MAX_LOADSTRING 100
#define DEFAULT_BUFLEN 		512
#define DEFAULT_PORT		"4200"
#define IDC_EDITCHAT		200
#define IDC_EDITESCRIBIR	201
#define IDC_EDITIP			202
#define IDC_BOTONENVIAR		203

char szMiIP[17] = "127.0.0.1";
char szUsuario[32] = "MaquinaWin";

DWORD WINAPI Servidor(LPVOID argumento);
int Cliente(HWND hChat, char* szDirIP, LPSTR pstrMensaje);

void EnviarMensaje(HWND hChat, HWND hEscribir, HWND hIP);
void Mostrar_Mensaje(HWND hChat, char* szMiIP, char* szUsuario, char*szMsg, COLORREF color);
void Colorear_texto(HWND hChat, char* szUsuario, long iLength, COLORREF color);

/* This is where all the input to the window goes to */
LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	static HWND hChat, hEscribir, hEnviar, hIP;
	HFONT hFont;
	static HANDLE hHiloServidor;
	static DWORD idHiloServidor;
	
	switch(Message) {
		case WM_CREATE: {
			LoadLibrary("riched20.dll");
			hChat = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, "", WS_CHILD
				| WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_READONLY
				| ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				0, 0, 0, 0,
				hwnd, (HMENU)IDC_EDITCHAT, (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
				
			hEscribir = CreateWindowEx(WS_EX_CLIENTEDGE, RICHEDIT_CLASS, "", WS_CHILD
				| WS_VISIBLE | WS_VSCROLL | WS_HSCROLL 
				| ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
				0, 0, 0, 0,
				hwnd, (HMENU)IDC_EDITESCRIBIR, (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
			
			hIP = CreateWindowEx(0, "EDIT", "", ES_LEFT | WS_CHILD
				| WS_VISIBLE | WS_BORDER | WS_TABSTOP,
				10, 350, 350, 30,
				hwnd, (HMENU)IDC_EDITIP, (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
				
			hEnviar = CreateWindowEx(0, "BUTTON", "Enviar",
				BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE |
				WS_BORDER | WS_TABSTOP,
				400, 350, 60, 30,
				hwnd, (HMENU)IDC_BOTONENVIAR, (HINSTANCE) GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
				
			hFont = CreateFont(18, 0, 0, 0, 700, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
			
			InvalidateRect(hChat, NULL, TRUE);
			SendMessage(hChat, WM_SETFONT, (WPARAM)hFont, 0);
			SetFocus(hEscribir);
			
			hHiloServidor = CreateThread(NULL, 0, Servidor,
				(LPVOID)hChat, 0, &idHiloServidor);
			
			if (hHiloServidor == NULL) {
				MessageBox(hwnd, "Error al crear el hilo servidor", "Error", MB_OK | MB_ICONERROR);
			}
			
			break;
		}
		
		case WM_SIZE: {
			MoveWindow(hChat, 1, 1, LOWORD(lParam) - 1, HIWORD(lParam) - 140, TRUE);
			MoveWindow(hEscribir, 1, HIWORD(lParam) - 135, LOWORD(lParam) - 1, 60, TRUE);
			MoveWindow(hIP, 1, HIWORD(lParam) - 70, LOWORD(lParam) - 101, 30, TRUE);
			MoveWindow(hEnviar, LOWORD(lParam) - 70, HIWORD(lParam) - 70, 60, 30, TRUE);
			break;
		}
		
		case WM_COMMAND: {
			int wmId = LOWORD(wParam);
			
			switch(wmId) {
				case IDC_BOTONENVIAR: 
					EnviarMensaje(hChat, hEscribir, hIP);
					break;
			}
			break;
		}
		
		/* Upon destruction, tell the main thread to stop */
		case WM_DESTROY: {
			CloseHandle(hHiloServidor);
			PostQuitMessage(0);
			break;
		}
		
		/* All other messages (a lot of them) are processed using default procedures */
		default:
			return DefWindowProc(hwnd, Message, wParam, lParam);
	}
	return 0;
}

/* The 'main' function of Win32 GUI programs: this is where execution starts */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc; /* A properties struct of our window */
	HWND hwnd; /* A 'HANDLE', hence the H, or a pointer to our window */
	MSG msg; /* A temporary location for all messages */
	
	LPSTR tchIp[17], tchszUsuario[32];
	if (_tcslen(lpCmdLine) > 10) {
		//swscanf(lpCmdLine, "%s %s", tchIp, tchszUsuario);
		sscanf(lpCmdLine, "%s %s", tchIp, tchszUsuario);
		sprintf(szMiIP, "%s", tchIp);
		sprintf(szUsuario, "%s", tchszUsuario);
	}

	/* zero out the struct and set the stuff we want to modify */
	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc; /* This is where we will send messages to */
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	/* White, COLOR_WINDOW is just a #define for a system color, try Ctrl+Clicking it */
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "WindowClass";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION); /* Load a standard icon */
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION); /* use the name "A" to use the project icon */

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"WindowClass","Caption",WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, /* x */
		CW_USEDEFAULT, /* y */
		500, /* width */
		450, /* height */
		NULL,NULL,hInstance,NULL);

	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	/*
		This is the heart of our program where all input is processed and 
		sent to WndProc. Note that GetMessage blocks code flow until it receives something, so
		this loop will not produce unreasonably high CPU usage
	*/
	while(GetMessage(&msg, NULL, 0, 0) > 0) { /* If no error is received... */
		TranslateMessage(&msg); /* Translate key codes to chars if present */
		DispatchMessage(&msg); /* Send it to WndProc */
	}
	return msg.wParam;
}


int Cliente(HWND hChat, char* szDirIP, LPSTR pstrMensaje) {
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	int iResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char szMsg[256];
	char localhost[] = "localhost";
	char chat[] = "chat";
	TCHAR msgFalla[256];
	
	//SetWindowText(hChat, szUsuario);
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		wsprintf(msgFalla, "WSAStartup failed with error: %d\n", iResult);
		MessageBox(NULL, msgFalla, "Error en cliente", MB_OK | MB_ICONERROR);
		
		return 1;
	}
	
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	iResult = getaddrinfo(szDirIP, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		wsprintf(msgFalla, "getaddrinfo failed with error: %d\n", iResult);
		MessageBox(NULL, msgFalla, "Error en cliente", MB_OK | MB_ICONERROR);
		WSACleanup();
		
		return 1;
	}
	
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		
		if (ConnectSocket == INVALID_SOCKET) {
			wsprintf(msgFalla, "socket failed with error: %d\n", WSAGetLastError());
			MessageBox(NULL, msgFalla, "Error en cliente", MB_OK | MB_ICONERROR);
			WSACleanup();
			
			return 1;
		}
		
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		
		break;
	}
	
	freeaddrinfo(result);
	
	if (ConnectSocket == INVALID_SOCKET) {
		MessageBox(NULL, "Unable to connect to server!\n", "Error en  cliente", MB_OK | MB_ICONERROR);
		sprintf(szMsg, "Error en la llamada a connect\nla dirección %s no es válida", szDirIP);
		Mostrar_Mensaje(hChat, localhost, chat, szMsg, RGB(255, 0, 0));
		WSACleanup();
		
		return 1;
	}
	
	sprintf(szMsg, "%s %s", szDirIP, szUsuario);
	
	iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);
	iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);
	
	strcpy(szMsg, pstrMensaje);	
	
	iResult = send(ConnectSocket, szMsg, sizeof(char) * 256, 0);
	iResult = shutdown(ConnectSocket, SD_SEND);
	iResult = recv(ConnectSocket, szMsg, sizeof(char) * 256, 0);
	
	//sprintf(prueba, "ojo %s", szMsg);
	//SetWindowText(hChat, prueba);
	Mostrar_Mensaje(hChat, szMiIP, szUsuario, szMsg, RGB(0, 0, 255));
	
	closesocket(ConnectSocket);
	WSACleanup();
	
	return 1;
}


DWORD WINAPI Servidor(LPVOID argumento) {
	HWND hChat = (HWND)argumento;
	WSADATA wsaData;
	int iResult;
	TCHAR msgFalla[256];
	
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;
	
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	
	int iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;
	char szBuffer[256], szIP[16], szNN[32];
	
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		wsprintf(msgFalla, "WSAStartup failed with error: %d\n", iResult);
		MessageBox(NULL, msgFalla, "Error em servidore", MB_OK | MB_ICONERROR);
		
		return 1;
	}
	
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		wsprintf(msgFalla, "getaddrinfo failed with error: %d", iResult);
		MessageBox(NULL, msgFalla, "Error en servidor", MB_OK | MB_ICONERROR);
		WSACleanup();
		
		return 1;
	}
	
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		wsprintf(msgFalla, "socket failed with error: %d", iResult);
		MessageBox(NULL, msgFalla, "Error en servidor", MB_OK | MB_ICONERROR);
		freeaddrinfo(result);
		WSACleanup();
		
		return 1;
	}
	
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		wsprintf(msgFalla, "listen failed with error: %d\n", WSAGetLastError());
		MessageBox(NULL, msgFalla, "Error en servidor", MB_OK | MB_ICONERROR);
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		
		return 1;
	}
	
	freeaddrinfo(result);
	
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		wsprintf(msgFalla, "listen failed with error: %d", WSAGetLastError());
		MessageBox(NULL, msgFalla, "Error en servidor", MB_OK | MB_ICONERROR);
		closesocket(ListenSocket);
		WSACleanup();
		
		return 1;
	}
	
	while (TRUE) {
		//MessageBox(NULL, "Esperando conexión", "Depuración", MB_OK);
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) {
			wsprintf(msgFalla, "acept failed with error: %d", iResult);
			MessageBox(NULL, msgFalla, "Error en servidor", MB_OK | MB_ICONERROR);
			closesocket(ListenSocket);
			WSACleanup();
		}
		
		iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);
		sscanf(szBuffer, "%s %s", szIP, szNN);
		sprintf(szBuffer, "Ok");
		
		iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);
		iResult = recv(ClientSocket, szBuffer, sizeof(char) * 256, 0);
		iSendResult = send(ClientSocket, szBuffer, sizeof(char) * 256, 0);
		
		Mostrar_Mensaje(hChat, szIP, szNN, szBuffer, RGB(34, 177, 76));
		
		iResult = shutdown(ClientSocket, SD_SEND);
	}
	
	closesocket(ClientSocket);
	WSACleanup();
}

void Mostrar_Mensaje(HWND hChat, char* ip, char* szUsuario, char* szMsg, COLORREF color) {
	long iLength;
	char* pstrBuffer = NULL;
	wchar_t* ptchBuffer = NULL;
	DWORD iIni = 0, iFin = 0;
	POINT posScroll;
	int r = 0;
	size_t i;
	
	int tam = strlen(szUsuario) + strlen(ip) + strlen(szMsg);
	
	if ((pstrBuffer = (LPSTR)malloc(sizeof(char) * (tam + 10))) == NULL ||
		(ptchBuffer = (wchar_t*)malloc(sizeof(wchar_t) * (tam + 10))) == NULL)
			MessageBox(NULL, "Error al reservar memoria", "Error", MB_OK | MB_ICONERROR);
			
	//snprintf(pstrBuffer, tam + 9, "%s -> %s\n%s\n", szUsuario, ip, szMsg);
	sprintf(pstrBuffer, "%s -> %s\n%s\n", szUsuario, ip, szMsg);
	tam = strlen(pstrBuffer);
	/*******************************************************************************/
	//mbstowcs(ptchBuffer, pstrBuffer, tam + 1);
	
	//SetWindowText(hChat, pstrBuffer);
	
	iLength = GetWindowTextLength(hChat);
	r = SendMessage(hChat, EM_GETLINECOUNT, 0, 0) - 1;
	
	SendMessage(hChat, EM_SETSEL, (WPARAM)iLength, (LPARAM)iLength + tam);
	SendMessage(hChat, EM_REPLACESEL, FALSE, (LPARAM)pstrBuffer);
	
	Colorear_texto(hChat, szUsuario, iLength - r, color);
	
	SetFocus(hChat);
	SendMessage(hChat, EM_GETSEL, (WPARAM)&iIni, (LPARAM)&iFin);
	SendMessage(hChat, EM_GETSCROLLPOS, 0L, (LPARAM)&posScroll);
	SendMessage(hChat, EM_SETSEL, (WPARAM)iIni, (LPARAM)iFin);
	SendMessage(hChat, EM_SETSCROLLPOS, 0L, (LPARAM)&posScroll);
	
	free(pstrBuffer);
	free(ptchBuffer);
}

void Colorear_texto(HWND hChat, char* szUsuario, long iLength, COLORREF color) {
	CHARFORMAT2 cf;
	size_t i;
	
	memset(&cf, 0, sizeof cf);
	cf.cbSize = sizeof(CHARFORMAT2);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = color;
	
	LPSTR auxiliar;
	if ((auxiliar = (LPSTR)malloc(sizeof(char) * (35 + 2))) == NULL)
		MessageBox(NULL, "Error al reservar memoria", "Error", MB_OK | MB_ICONERROR);
		
	int tam = strlen(szUsuario);
	//mbstowcs(auxiliar, szUsuario, sizeof(szUsuario) + 2);
	strcpy(auxiliar, szUsuario);
	
	SendMessage(hChat, EM_SETSEL, (WPARAM)iLength, (LPARAM)iLength + tam);
	SendMessage(hChat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	SendMessage(hChat, EM_REPLACESEL, FALSE, (LPARAM)auxiliar);
	cf.crTextColor = RGB(0, 0, 0);
}

void EnviarMensaje(HWND hChat, HWND hEscribir, HWND hIP) {
	LPSTR tchDirIP;
	char szDirIP[16];
	int tam = 0;
	size_t i;
	
	if ((tchDirIP = (LPSTR)malloc(sizeof(char) * (16 + 2))) == NULL)
		MessageBox(NULL, "Error al reservar memoria", "Error", MB_OK | MB_ICONERROR);
	
	GetWindowText(hIP, tchDirIP, 16);
	tam = GetWindowTextLength(hIP);
	tchDirIP[tam] = '\0';
	
	long iLength;
	LPSTR pstrBuffer;
	wchar_t* ptchBuffer;
	
	iLength = GetWindowTextLength(hEscribir);
	if ((pstrBuffer = (LPSTR)malloc(sizeof(char) * (iLength + 2))) == NULL ||
		(ptchBuffer = (wchar_t*)malloc(sizeof(wchar_t) * (iLength + 2))) == NULL)
			MessageBox(NULL, "Error al reservar memoria", "Error", MB_OK | MB_ICONERROR);
	else {
		GetWindowTextA(hEscribir, pstrBuffer, iLength + 1);
		pstrBuffer[iLength + 1] = '\0';
		
		Cliente(hChat, tchDirIP, pstrBuffer);
		
		SetWindowText(hEscribir, "");
		free(pstrBuffer);
		free(ptchBuffer);
	}
}
