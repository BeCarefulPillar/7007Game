#ifndef _EASY_TCP_CLIENT_HPP_
#define _EASY_TCP_CLIENT_HPP_

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

#ifndef REVC_BUFF_SIZE
#define REVC_BUFF_SIZE 10240 * 5
#endif // !REVC_BUFF_SIZE

#include <stdio.h>
#include "MessageHeader.hpp"
class EasyTcpClient {
    SOCKET _sock;
    bool _isConnect;
public:
    EasyTcpClient() {
        _sock = INVALID_SOCKET;
        _isConnect = false;
    }
    virtual ~EasyTcpClient() {
        Close();
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
            //printf("socket success \n");
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
            _isConnect = true;
            //printf("<sock = %d> 连接 <%s:%d>成功 \n", (int)_sock, ip, port);
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
        _isConnect = false;
    }
    //运行select
    bool OnRun() {
        if (!IsRun()) {
            return false;
        }
        fd_set fdRead;
        FD_ZERO(&fdRead);
        FD_SET(_sock, &fdRead);

        timeval t = { 0, 10 };
        int ret = select((int)_sock + 1, &fdRead, 0, 0, nullptr);
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
        return _sock != INVALID_SOCKET && _isConnect;
    }

    char _szMsgBuf[REVC_BUFF_SIZE] = {}; //第二缓冲区，消息缓冲区
    int _lastPos = 0;//消息缓冲区结尾
    //处理粘包，拆包
    int RecvData() {
        char* szRevc = _szMsgBuf + _lastPos;
        int nLen = (int)recv(_sock, szRevc, REVC_BUFF_SIZE - _lastPos, 0);
        if (nLen <= 0) {
            printf("服务器已经关闭 \n");
            Close();
            return -1;
        }
        //收到数据加入缓冲区
        //memcpy(_szMsgBuf + _lastPos, _szRevc, nLen);
        //消息缓冲区数据尾部向后
        _lastPos += nLen;
        //判断消息长度大于消息头
        while (_lastPos >= sizeof(DataHeader)) {
            //当前消息
            DataHeader *hd = (DataHeader*)_szMsgBuf;
            if (_lastPos >= hd->dataLen) {
                int msgLen = hd->dataLen;
                //处理消息
                OnNetMsg(hd);
                //数据前移
                memcpy(_szMsgBuf, _szMsgBuf + msgLen, _lastPos - msgLen);
                //消息尾部前移
                _lastPos -= msgLen;
            } else {
                break;
            }
        }

        return 0;
    }
    //响应网络消息
    virtual void OnNetMsg(DataHeader* hd) {
        if (!hd) {
            return;
        }
        switch (hd->cmd) {
        case CMD_LOGIN_RESULT: {
            LoginResult *loginRes = (LoginResult *)hd;
            printf("recv CMD_LOGIN_RESULT dataLen = %d, %s \n", loginRes->dataLen, loginRes->data);
        } break;
        case CMD_LOGOUT_RESULT: {
            LogoutResult *logoutData = (LogoutResult *)hd;
            //printf("recv CMD_LOGOUT_RESULT dataLen = %d\n", logoutData->dataLen);
        }break;
        case CMD_NEW_CLIENT_JOIN: {
            NewClientJoin *newClient = (NewClientJoin *)hd;
            //printf("recv CMD_NEW_CLIENT_JOIN dataLen = %d sock = %d\n", newClient->dataLen, newClient->sock);
        }break;
        case CMD_ERROR: {
            printf("recv CMD_ERROR dataLen = %d  \n", hd->dataLen);
        }break;
        default:{
            printf("recv 未知消息 dataLen = %d  \n", hd->dataLen);
        }
        }
    }

    int SendData(DataHeader* hd, int nLen) {
        if (IsRun() && hd) {
            return send(_sock, (const char*)hd, nLen, 0);
        }
        return SOCKET_ERROR;
    }



private:

};

#endif