#include <iostream>

#define WIN32_LEAN_AND_MEAN //避免windows.h 和 WinSock2.h 中的宏定义重复
#include <windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

//静态链接库 win平台
//#pragma comment(lib, "ws2_32.lib")

int main()
{
    WORD ver = MAKEWORD(2, 2); //socket版本 2.x环境
    WSADATA data;               
    WSAStartup(ver, &data);
    //---------------------------
    //-socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //ipv4, 流数据, tcp
    //-bind-
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567); //主机数据转换到网络数据 host to net unsigned short
    _sin.sin_addr.S_un.S_addr = inet_pton(AF_INET, "127.0.0.1", &_sin.sin_addr); //INADDR_ANY; // ip
    if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(_sin))) {
        printf("bind error \n");
    } else {
        printf("bind success \n");
    }
    //-listen-
    if (SOCKET_ERROR == listen(_sock, 5)) {//监听人数 
        printf("listen error \n");
    } else {
        printf("listen success \n");
    }
    //-accept-
    sockaddr_in clientAddr = {};
    int nAddrLen = sizeof(clientAddr);
    SOCKET _cSock = INVALID_SOCKET; //无效socket

    char msgBuff[] = "Hello, server";
    while (true) {
        _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
        if (INVALID_SOCKET == _cSock) {
            printf("error, client invalid socket");
        }

        char str[INET_ADDRSTRLEN];
        printf("new client add: ip = %s /n", inet_ntop(AF_INET, &clientAddr.sin_addr, str, sizeof(str)));
        //-send-
        send(_cSock, msgBuff, strlen(msgBuff) + 1, 0);
    }
    
    //-close-
    closesocket(_sock);
    //---------------------------
    WSACleanup();
    return 0;
}
