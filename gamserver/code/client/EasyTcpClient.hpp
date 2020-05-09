#pragma once
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN //避免windows.h 和 WinSock2.h 中的宏定义重复
#include <windows.h>
#include <WinSock2.h>
//静态链接库 win平台
//#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h> //uni std
#include <arpa/inet.h>
#include <string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#include <stdio.h>
#include "MessageHeader.hpp"
class EasyTcpClient {
    
public:
    SOCKET _sock;
    EasyTcpClient() {
        _sock = INVALID_SOCKET;
    }
    //初始化socket
    void InitSocket() {
        if (INVALID_SOCKET != _sock) {
            Close();
        }
#ifdef _WIN32
        WORD ver = MAKEWORD(2, 2); //socket版本 2.x环境
        WSADATA data;
        WSAStartup(ver, &data);
#endif
        _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //ipv4, 流数据, tcp
        if (INVALID_SOCKET == _sock) {
            printf("socket error \n");
        } else {
            printf("socket success \n");
        }
    }
    //连接服务器
    int Connet(const char* ip, unsigned short port) {
        if (INVALID_SOCKET == _sock) {
            InitSocket();
        }
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port);
#ifdef _WIN32
        _sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
        _sin.sin_addr.s_addr = inet_addr(ip);
#endif
        if (SOCKET_ERROR == connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in))) {
            printf("connect error \n");
            Close();
            return -1;
        } else {
            printf("<sock = %d> 连接 <%s:%d>成功 \n", (int)_sock, ip, port);
            return 0;
        }
    }
    //关闭连接
    void Close() {
        if (INVALID_SOCKET == _sock) {
            return;
        }
#ifdef _WIN32
        //7close-
        closesocket(_sock);
        WSACleanup();
#else
        close(_sock);
#endif
        _sock = INVALID_SOCKET;
    }
    //运行select
    bool OnRun() {
        fd_set fdRead;
        FD_ZERO(&fdRead);
        FD_SET(_sock, &fdRead);

        timeval t = { 1,0 };
        int ret = select((int)_sock + 1, &fdRead, 0, 0, &t);
        if (ret < 0) {
            printf("客户端关闭1 \n");
            Close();
            return false;
        }
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);
            ret = RecvData();
            if (-1 == ret) {
                printf("客户端关闭2 \n");
                Close();
                return false;
            }
        }
        return true;
    }

    bool IsRun() {
        return _sock != INVALID_SOCKET;
    }

    //处理粘包，拆包
    int RecvData() {
        char szRevc[1024]; //加一个缓冲区
        int nLen = (int)recv(_sock, szRevc, sizeof(DataHeader), 0);
        DataHeader *hd = (DataHeader*)szRevc;
        if (nLen <= 0) {
            printf("server exist out\n");
            Close();
            return -1;
        }

        recv(_sock, szRevc + sizeof(DataHeader), hd->dataLen - sizeof(DataHeader), 0);
        OnNetMsg(hd);
        return 0;
    }

    void OnNetMsg(DataHeader* hd) {
        if (!hd) {
            return;
        }
        switch (hd->cmd) {
        case CMD_LOGIN_RESULT: {
            LoginResult *loginRes = (LoginResult *)hd;
            printf("recv CMD_LOGIN_RESULT dataLen = %d, \n", loginRes->dataLen);
        } break;
        case CMD_LOGOUT_RESULT: {
            LogoutResult *logoutData = (LogoutResult *)hd;
            printf("recv CMD_LOGOUT_RESULT dataLen = %d\n", logoutData->dataLen);
        }break;
        case CMD_NEW_CLIENT_JOIN: {
            NewClientJoin *newClient = (NewClientJoin *)hd;
            printf("recv CMD_NEW_CLIENT_JOIN dataLen = %d sock = %d\n", newClient->dataLen, newClient->sock);
        }break;
        }
    }

    int SendData(DataHeader* hd) {
        if (IsRun() && hd) {
            return send(_sock, (const char*)hd, hd->dataLen, 0);
        }
        return SOCKET_ERROR;
    }

    virtual ~EasyTcpClient() {
        Close();
    }

private:

};

