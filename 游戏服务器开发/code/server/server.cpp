#include <iostream>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN //避免windows.h 和 WinSock2.h 中的宏定义重复
#include <windows.h>
#include <WinSock2.h>

//静态链接库 win平台
//#pragma comment(lib, "ws2_32.lib")

enum CMD {
    CMD_ERROR = 0,
    CMD_LOGIN,
    CMD_LOGOUT,
};

struct dataHeader {
    int cmd;
    int dataLen;
};

//DataPackage
struct login {
    char account[32];
    char password[32];
};

struct loginResult {
    int res;
};

struct logout {
    char account[32];
};

struct logoutResult {
    int res;
};

int main() {
    WORD ver = MAKEWORD(2, 2); //socket版本 2.x环境
    WSADATA data;
    WSAStartup(ver, &data);
    //---------------------------
    //-socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //ipv4, 流数据, tcp
    if (INVALID_SOCKET == _sock) {
        printf("socket error \n");
    } else {
        printf("socket success \n");
    }
    //-bind-
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567); //主机数据转换到网络数据 host to net unsigned short
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); //INADDR_ANY; // ip
    if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in))) {
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
    int nAddrLen = sizeof(sockaddr_in);
    SOCKET _cSock = INVALID_SOCKET; //无效socket

    _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
    if (INVALID_SOCKET == _cSock) {
        printf("error, client invalid socket \n");
    }
    printf("new client add: _cSock = %d, ip = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));

    while (true) {
        //recv client data
        dataHeader hd = {};
        int nLen = recv(_cSock, (char *)&hd, sizeof(dataHeader), 0);
        printf("recv cmd = %d, dataLen %d \n", hd.cmd, hd.dataLen);

        switch (hd.cmd) {
        case CMD_LOGIN: {
            login loginData = {};
            recv(_cSock, (char *)&loginData, sizeof(login), 0);
            printf("recv account = %s,password %s \n", loginData.account, loginData.password);

            send(_cSock, (const char *)&hd, sizeof(dataHeader), 0);
            loginResult loginRes = { 0 };
            send(_cSock, (const char *)&loginRes, sizeof(loginResult), 0);
        } break;
        case CMD_LOGOUT: {
            logout logoutData = {};
            recv(_cSock, (char *)&logoutData, sizeof(logout), 0);
            printf("recv account = %s\n", logoutData.account);

            send(_cSock, (const char *)&hd, sizeof(dataHeader), 0);
            logoutResult logoutRes = { 0 };
            send(_cSock, (const char *)&logoutRes, sizeof(logoutResult), 0);
        }break;

        default: {
            hd.cmd = CMD_ERROR;
            send(_cSock, (const char *)&hd, sizeof(dataHeader), 0);
        }break;
        }
    }

    //-close-
    closesocket(_sock);
    //---------------------------
    WSACleanup();

    printf("server exist out\n");
    return 0;
}
