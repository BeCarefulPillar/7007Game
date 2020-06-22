#ifndef _EASY_TCP_SERVER_HPP_
#define _EASY_TCP_SERVER_HPP_
#include "Cell.hpp"
#include "INetEvent.hpp"
#include "CellServer.hpp"
#include "CellClient.hpp"

#include <stdio.h>
#include <thread>
#include <mutex> //锁
#include <atomic>
#include <vector>

class EasyTcpServer: public INetEvent {
private:
    SOCKET _sock;
    std::atomic_int _clientCount; 
    std::vector<CellServer *> _cellServer;//使用指针的原因是栈空间只有1M到2M
    CellTimestame _tTime;
    std::atomic_int _msgCount;
    std::atomic_int _recvCount;
public:
    EasyTcpServer() {
        _sock = INVALID_SOCKET;
        _msgCount = 0;
        _recvCount = 0;
        _clientCount = 0;
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
    //开启服务线程
    void Start() {
        for (int i = 0; i < CELL_SERVER_THEARD; i++) {
            CellServer* server = new CellServer(_sock);
            _cellServer.push_back(server);
            //注册网络事件
            server->SetNetObj(this);
            server->Start();
        }
    }

    //接受客户端连接
    SOCKET Accept(){
        sockaddr_in clientAddr = {};
        int nAddrLen = sizeof(sockaddr_in);
        SOCKET cSock = INVALID_SOCKET; //无效socket
#ifdef _WIN32
        cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
        cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
        if (INVALID_SOCKET == cSock) {
            printf("客户端连接失败 \n");
        }

        AddClientToCellServer(new CellClient(cSock, clientAddr));
        //printf("新客户端加入: cSock = %d,客户端数量 = %d, ip = %s \n", (int)cSock, _clientCount, inet_ntoa(clientAddr.sin_addr));
        return cSock;
    }

    void AddClientToCellServer(CellClient* pClient) {
        auto minSer = _cellServer[0];
        for (auto ser : _cellServer)
        {
            if (minSer->GetClientCount() > ser->GetClientCount()) {
                minSer = ser;
            }
        }
        minSer->AddClient(pClient);
        OnNetJoin(pClient);
    }

    //关闭socket
    void Close() {
        if (INVALID_SOCKET == _sock) {
            return;
        }
        for (int i = 0; i < (int)_cellServer.size(); i++) {
            delete _cellServer[i];
        }
        _cellServer.clear();

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
        Time4Msg();
        fd_set fdRead;
        //fd_set fdWrite;
        //fd_set fdExcept;
        FD_ZERO(&fdRead);
        //FD_ZERO(&fdWrite);
        //FD_ZERO(&fdExcept);

        FD_SET(_sock, &fdRead);
        //FD_SET(_sock, &fdWrite);
        //FD_SET(_sock, &fdExcept);
        //伯克利 socket
        /*param
        1.在windows上没有意义, fd_set 所有集合中 描述符(socket)范围, 而不是数量
        2.可读集合
        3.可写集合
        4.异常集合
        5.超时时间
        */

        timeval time = { 0, 10 };//加入这个参数为null 可以当做一个必须要客户端请求的服务器
        int ret = select(_sock + 1, &fdRead, nullptr, nullptr, &time); //select 性能瓶颈 最大的集合只有64
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
        return true;
    }

    //是否工作中
    bool IsRun() {
        return _sock != INVALID_SOCKET;
    }

    //响应网络消息
    void Time4Msg() {
        auto t1 = _tTime.GetElapsedSecond();
        if (t1 > 1.0) {
            printf("thread<%d> ,time <%lf>, socket <%d>, client <%d>, recvCount<%d>, msg<%d>\n", (int)_cellServer.size(), t1, (int)_sock, (int)_clientCount, (int)_recvCount, (int)_msgCount);
            _tTime.Update();
            _recvCount = 0;
            _msgCount = 0;
        }
    }

    virtual void OnNetJoin(CellClient* pClient) {
        _clientCount++;
    }

    virtual void OnNetLevel(CellClient* pClient) {
        _clientCount--;
    }

    virtual void OnNetRecv(CellClient* pClient) {
        _recvCount++;
    }

    virtual void OnNetMsg(CellServer* pCellServer ,CellClient* pClient, DataHeader* pHd) {
        _msgCount++;
        if (!pHd) {
            return;
        }
        switch (pHd->cmd) {
        case CMD_LOGIN: {
            Login *loginData = (Login *)pHd;
            /*printf("recv <socket = %d> ,CMD_LOGIN dataLen = %d,account = %s,password=%s \n", pClient->GetSocket(), loginData->dataLen, loginData->account, loginData->password);*/
            LoginResult loginRes;
            pClient->SendData(&loginRes);
//             LoginResult* loginRes =new LoginResult();
//             pCellServer->AddSendTask(pClient, loginRes);
        } break;
        case CMD_LOGOUT: {
            Logout *logoutData = (Logout *)pHd;
            //printf("recv <socket = %d>, CMD_LOGOUT dataLen = %d, account = %s\n", cSock, logoutData->dataLen, logoutData->account);

            //LogoutResult logoutRes;

            //pClient->SendData(&logoutRes);
        }break;
        case CMD_HEART: {
            pClient->ResetDTHeart();
            HeartResult heartRes;
            pClient->SendData(&heartRes);
        }break;
        default: {
            DataHeader head;
            pClient->SendData(&head);
        }break;
        }
    }
};


#endif // !_EASY_TCP_SERVER_HPP_
