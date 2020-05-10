#ifndef _EASY_TCP_SERVER_HPP_
#define _EASY_TCP_SERVER_HPP_
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
#include <vector>
#include "MessageHeader.hpp"

class EasyTcpServer {
    SOCKET _sock;
    std::vector<SOCKET> g_client;
public:
    EasyTcpServer() {
        _sock = INVALID_SOCKET;
    }
    virtual ~EasyTcpServer() {
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
        //---------------------------
        //-socket
        _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //ipv4, 流数据, tcp
        if (INVALID_SOCKET == _sock) {
            printf("socket初始化错误 \n");
        } else {
            printf("socket初始化成功 \n");
        }
    }
    //绑定端口号
    int Bind(const char * ip, unsigned short port) {
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port); //主机数据转换到网络数据 host to net unsigned short
#ifdef _WIN32
        if (ip) {
            _sin.sin_addr.S_un.S_addr = inet_addr(ip); //INADDR_ANY; // ip
        } else {
            _sin.sin_addr.S_un.S_addr = INADDR_ANY;
        }
#else
        if (ip) {
            _sin.sin_addr.s_addr = inet_addr(ip); //INADDR_ANY; // ip
        } else {
            _sin.sin_addr.s_addr = INADDR_ANY;
        }
        
#endif
        int ret = bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
        if (SOCKET_ERROR == ret) {
            printf("连接端口<port = %d>错误 \n",port);
            Close();
        } else {
            printf("连接端口<port = %d> 成功 \n",port);
        }
        return ret;
    }
    //监听端口号
    int Listen(int n) {
        int ret = listen(_sock, n);
        if (SOCKET_ERROR == ret) {//监听人数 
            printf("监听错误 \n");
            Close();
        } else {
            printf("监听成功 \n");
        }
        return ret;
    }
    //接受客户端连接
    SOCKET Accept(){
        sockaddr_in clientAddr = {};
        int nAddrLen = sizeof(sockaddr_in);
        SOCKET _cSock = INVALID_SOCKET; //无效socket
#ifdef _WIN32
        _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
        _cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
        if (INVALID_SOCKET == _cSock) {
            printf("客户端连接失败 \n");
        }

        for (int i = 0; i < (int)g_client.size(); i++) {
            NewClientJoin newClientJoin;
            newClientJoin.sock = _cSock;
            SendData2All(&newClientJoin);
        }
        g_client.push_back(_cSock);
        printf("新客户端加入: _cSock = %d, ip = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
        return _cSock;
    }

    //关闭socket
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
    //处理网络消息
    bool OnRun() {
        if (!IsRun()) {
            return false;
        }
        fd_set fdRead;
        fd_set fdWrite;
        fd_set fdExcept;
        FD_ZERO(&fdRead);
        FD_ZERO(&fdWrite);
        FD_ZERO(&fdExcept);

        FD_SET(_sock, &fdRead);
        FD_SET(_sock, &fdWrite);
        FD_SET(_sock, &fdExcept);
        //伯克利 socket
        /*param
        1.在windows上没有意义, fd_set 所有集合中 描述符(socket)范围, 而不是数量
        2.可读集合
        3.可写集合
        4.异常集合
        5.超时时间
        */
        int maxSock = _sock;
        for (int i = 0; i < (int)g_client.size(); i++) {
            FD_SET(g_client[i], &fdRead);
            if (g_client[i] > _sock) {
                maxSock = g_client[i];
            }
        }

        timeval time = { 1, 0 };//加入这个参数为null 可以当做一个必须要客户端请求的服务器
        int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &time); //select 性能瓶颈 最大的集合只有64
        if (ret < 0) {
            printf("select 结束 \n");
            Close();
            return false;
        }
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);
            //-accept-
            Accept();
        }
        for (int i = 0; i < (int)g_client.size(); i++) {
            if (FD_ISSET(g_client[i], &fdRead)) {
                int ret = RecvData(g_client[i]);
                if (ret == -1) {
                    auto iter = g_client.begin() + i;
                    if (iter != g_client.end()) {
                        g_client.erase(iter);
                    }
                }
            }
        }
        return true;
    }

    //是否工作中
    bool IsRun() {
        return _sock != INVALID_SOCKET;
    }
    //接收数据 处理粘包 拆分包

    int RecvData(SOCKET _cSock) {
        char szRevc[1024]; //加一个缓冲区
        int nLen = recv(_cSock, szRevc, sizeof(DataHeader), 0);
        DataHeader *hd = (DataHeader*)szRevc;
        if (nLen <= 0) {
            printf("<sock = %d>, 客户端已经退出\n", _cSock);
            return -1;
        }

        recv(_cSock, szRevc + sizeof(DataHeader), hd->dataLen - sizeof(DataHeader), 0);
        OnNetMsg(hd, _cSock);
        return 0;
    }

    //响应网络消息
    virtual void OnNetMsg(DataHeader* hd, SOCKET _cSock) {
        if (!hd) {
            return;
        }
        switch (hd->cmd) {
        case CMD_LOGIN: {
            Login *loginData = (Login *)hd;
            printf("recv <socket = %d> ,CMD_LOGIN dataLen = %d,account = %s,password=%s \n", _cSock, loginData->dataLen, loginData->account, loginData->password);

            LoginResult loginRes;
            SendData(&loginRes, _cSock);
        } break;
        case CMD_LOGOUT: {
            Logout *logoutData = (Logout *)hd;
            printf("recv <socket = %d>, CMD_LOGOUT dataLen = %d, account = %s\n", _cSock, logoutData->dataLen, logoutData->account);

            LogoutResult logoutRes;

            SendData(&logoutRes, _cSock);
        }break;
        default: {
            DataHeader head = { CMD_ERROR , 0 };
            SendData(&head, _cSock);
        }break;
        }
    }

    //发送数据
    int SendData(DataHeader* hd, SOCKET _csock) {
        if (IsRun() && hd) {
            return send(_csock, (const char*)hd, hd->dataLen, 0);
        }
        return SOCKET_ERROR;
    }

    //对所有连接上的人发送数据
    void SendData2All(DataHeader* hd) {
        if (IsRun() && hd) {
            for (int i = 0; i < (int)g_client.size(); i++) {
                SendData(hd, g_client[i]);
            }
        }
    }


private:

};


#endif // !_EASY_TCP_SERVER_HPP_
