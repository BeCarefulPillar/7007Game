#include <iostream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN //避免windows.h 和 WinSock2.h 中的宏定义重复
#include <windows.h>
#include <WinSock2.h>

//静态链接库 win平台
//#pragma comment(lib, "ws2_32.lib")

struct DataPackage {
    int age;
    char name[32];
};

int main() {
    WORD ver = MAKEWORD(2, 2); //socket版本 2.x环境
    WSADATA data;
    WSAStartup(ver, &data);
    //---------------------------
    //1 socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //ipv4, 流数据, tcp
    if (INVALID_SOCKET == _sock) {
        printf("socket error \n");
    } else {
        printf("socket success \n");
    }
    //2 connect
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    if (SOCKET_ERROR == connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in))) {
        printf("connect error \n");
    } else {
        printf("connect error \n");
    }
    while (true) {
        char cmdBuf[128] = {};
        scanf_s("%s", cmdBuf, 128);
        //4 deal req
        if (0 == strcmp(cmdBuf, "exit")) {
            break;
        } else {
            //5 send
            send(_sock, cmdBuf, strlen(cmdBuf) + 1, 0);
        }
        //6 recv
        char recvBuf[256] = {};
        int nlen = recv(_sock, recvBuf, 256, 0);
        if (nlen > 0) {
            DataPackage *dp = (DataPackage *)recvBuf;
            printf("recv data age = %d , name = %s \n", dp->age, dp->name);
        }
    }

    
    //7close-
    closesocket(_sock);
    //---------------------------
    WSACleanup();
    printf("client exist out");
    getchar();
    return 0;
}
