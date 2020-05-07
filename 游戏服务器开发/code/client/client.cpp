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
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
};

struct dataHeader {
    int cmd;
    int dataLen;
};

//DataPackage
struct login : public dataHeader {
    login() {
        cmd = CMD_LOGIN;
        dataLen = sizeof(login);
    };
    char account[32];
    char password[32];
};

struct loginResult : public dataHeader {
    loginResult() {
        cmd = CMD_LOGIN_RESULT;
        dataLen = sizeof(loginResult);
        result = 0;
    };
    int result;
};

struct logout : public dataHeader {
    logout() {
        cmd = CMD_LOGOUT;
        dataLen = sizeof(logout);
    };
    char account[32];
};

struct logoutResult : public dataHeader {
    logoutResult() {
        cmd = CMD_LOGOUT_RESULT;
        dataLen = sizeof(logoutResult);
        result = 0;
    };
    int result;
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
        printf("connect succeed \n");
    }
    while (true) {
        char cmdBuf[128] = {};
        scanf_s("%s", cmdBuf, 128);
        //4 deal req
        if (0 == strcmp(cmdBuf, "exit")) {
            break;
        } else if (0 == strcmp(cmdBuf, "login")) {
            login loginData;
            strcpy_s(loginData.account, "ssss");
            strcpy_s(loginData.password, "123");
            //5 send
            send(_sock, (const char*)&loginData, sizeof(login), 0);
            //6 recv
            loginResult loginRes = {};
            recv(_sock, (char *)&loginRes, sizeof(loginResult), 0);
            printf("recv resDh = %d , loginRes = %d \n", loginRes.cmd, loginRes.result);
        } else if (0 == strcmp(cmdBuf, "logout")) {
            dataHeader dh = { CMD_LOGOUT, sizeof(logout) };
            logout logoutData = {};
            strcpy_s(logoutData.account, "ssss");
            //5 send
            send(_sock, (const char*)&logoutData, sizeof(logout), 0);
            //6 recv
            logoutResult logoutRes = {};
            recv(_sock, (char *)&logoutRes, sizeof(logoutResult), 0);
            printf("recv resDh = %d , logout = %d \n", logoutRes.cmd, logoutRes.result);
        } else {
            printf("print error \n");
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
