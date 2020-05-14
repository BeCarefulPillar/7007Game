#ifndef _EASY_TCP_SERVER_HPP_
#define _EASY_TCP_SERVER_HPP_
#ifdef _WIN32
#define FD_SETSIZE 1024
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
#define REVC_BUFF_SIZE 10240
#endif // !REVC_BUFF_SIZE

#ifndef CELL_SERVER_THEARD
#define CELL_SERVER_THEARD 4
#endif

#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex> //锁
#include <atomic>
#include "MessageHeader.hpp"
#include "CellTimestame.hpp"

class ClientSocket {
private:
    SOCKET _sock;
    char _szMsgBuf[REVC_BUFF_SIZE * 10]; //第二缓冲区，消息缓冲区
    int _lastPos;//消息缓冲区结尾
public:
    ClientSocket(SOCKET sock) {
        _sock = sock;
        _lastPos = 0;
        memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
    }
    
    SOCKET GetSocket() {
        return _sock;
    }

    char* MsgBuf() {
        return _szMsgBuf;
    }

    int GetLastPos() {
        return _lastPos;
    }

    void SetLastPos(int lastPos) {
        _lastPos = lastPos;
    }
};

class INetEvent {
public:
    virtual void OnLevel(ClientSocket *pSock) = 0;

};

class CellServer {
private:
    SOCKET _sock;
    //正式
    std::vector<ClientSocket *> _client;
    //缓冲
    std::vector<ClientSocket *> _clientBuff;
    std::mutex _mutex;
    std::thread* _pThread;
    INetEvent* _pNetEvent;
public:
    std::atomic_int _recvCount;

    CellServer(SOCKET socket = INVALID_SOCKET) {
        _sock = socket;
        _recvCount = 0;
        _pThread = nullptr;
        _pNetEvent = nullptr;
    }

    ~CellServer() {
        Close();
    }

    void SetNetObj(INetEvent* event) {
        _pNetEvent = event;
    }

    int GetClientCount() {
        return _clientBuff.size() + _client.size();
    }

    void AddClient(ClientSocket* pClient) {
        std::lock_guard<std::mutex> lock(_mutex);
        _clientBuff.push_back(pClient); //在线程中要使用，所以要加锁
    }

    void Start() {
        _pThread = new std::thread(std::mem_fun(&CellServer::OnRun), this);
        _pThread->detach();
    }

    //关闭socket
    void Close() {
        if (INVALID_SOCKET == _sock) {
            return;
        }
#ifdef _WIN32
        //7close-
        for (int i = 0; i < (int)_client.size(); i++) {
            closesocket(_client[i]->GetSocket());
            delete _client[i];
        }
        closesocket(_sock);
        WSACleanup();
#else
        for (int i = 0; i < (int)_client.size(); i++) {
            closesocket(_client[i]->GetSocket());
            delete _client[i];
        }
        close(_sock);
#endif
        _sock = INVALID_SOCKET;
        _client.clear();
    }

    bool OnRun() {
        while (IsRun()) {
            if (!_clientBuff.empty()){
                std::lock_guard<std::mutex> lock(_mutex);
                for (auto pClient : _clientBuff){
                    _client.push_back(pClient);
                }
                _clientBuff.clear();
            }

            if (_client.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            fd_set fdRead;
            FD_ZERO(&fdRead);

            int maxSock = _client[0]->GetSocket();
            for (int i = 0; i < (int)_client.size(); i++) {
                FD_SET(_client[i]->GetSocket(), &fdRead);
                if (_client[i]->GetSocket() > _sock) {
                    maxSock = _client[i]->GetSocket();
                }
            }

            int ret = select(maxSock + 1, &fdRead, nullptr, nullptr, nullptr); //select 性能瓶颈 最大的集合只有64
            if (ret < 0) {
                printf("select 结束 \n");
                Close();
                return false;
            }

            for (int i = 0; i < (int)_client.size(); i++) {
                if (FD_ISSET(_client[i]->GetSocket(), &fdRead)) {
                    int ret = RecvData(_client[i]);
                    if (ret == -1) {
                        auto iter = _client.begin() + i;
                        if (iter != _client.end()) {
                            if (_pNetEvent) {
                                _pNetEvent->OnLevel(_client[i]);
                            }
                            delete _client[i];
                            _client.erase(iter);
                        }
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
    char _szRevc[REVC_BUFF_SIZE]; //加一个缓冲区
    int RecvData(ClientSocket* client) {
        _recvCount++;
        int nLen = recv(client->GetSocket(), _szRevc, REVC_BUFF_SIZE, 0);
        if (nLen <= 0) {
            printf("<sock = %d>, 客户端已经退出\n", client->GetSocket());
            return -1;
        }

        //收到数据加入缓冲区
        memcpy(client->MsgBuf() + client->GetLastPos(), _szRevc, nLen);
        //消息缓冲区数据尾部向后
        client->SetLastPos(client->GetLastPos() + nLen);
        //判断消息长度大于消息头
        while (client->GetLastPos() >= sizeof(DataHeader)) {
            //当前消息
            DataHeader *hd = (DataHeader*)client->MsgBuf();
            if (client->GetLastPos() >= hd->dataLen) {
                int msgLen = hd->dataLen;
                //处理消息
                OnNetMsg(hd, client->GetSocket());
                //数据前移
                memcpy(client->MsgBuf(), client->MsgBuf() + msgLen, client->GetLastPos() - msgLen);
                //消息尾部前移
                client->SetLastPos(client->GetLastPos() - msgLen);
            } else {
                break;
            }
        }

        return 0;
    }

    virtual void OnNetMsg(DataHeader* hd, SOCKET cSock) {
        if (!hd) {
            return;
        }
        switch (hd->cmd) {
        case CMD_LOGIN: {
            Login *loginData = (Login *)hd;
            //printf("recv <socket = %d> ,CMD_LOGIN dataLen = %d,account = %s,password=%s \n", cSock, loginData->dataLen, loginData->account, loginData->password);

            //LoginResult loginRes;
            //SendData(&loginRes, cSock);
        } break;
        case CMD_LOGOUT: {
            Logout *logoutData = (Logout *)hd;
            //printf("recv <socket = %d>, CMD_LOGOUT dataLen = %d, account = %s\n", cSock, logoutData->dataLen, logoutData->account);

            //LogoutResult logoutRes;

            //SendData(&logoutRes, cSock);
        }break;
        default: {
            DataHeader head;
            SendData(&head, cSock);
        }break;
        }
    }

    int SendData(DataHeader* hd, SOCKET cSock) {
        if (IsRun() && hd) {
            return send(cSock, (const char*)hd, hd->dataLen, 0);
        }
        return SOCKET_ERROR;
    }
};

class EasyTcpServer:public INetEvent {
private:
    SOCKET _sock;
    std::vector<ClientSocket *> _client; //使用指针的原因是栈空间只有1M到2M
    std::vector<CellServer *> _cellServer;
    CellTimestame _tTime;
public:
    int _recvCount;
    EasyTcpServer() {
        _sock = INVALID_SOCKET;
        _recvCount = 0;
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
            server->SetNetObj(this);
            server->Start();
        }
    }

    void OnLevel(ClientSocket* pClient) {
        for (int i = 0; i < _client.size(); i++) {
            if (_client[i] == pClient) {
                auto iter = _client.begin() + i;
                if (iter != _client.end()) {
                    _client.erase(iter);
                }
            }
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

//         for (int i = 0; i < (int)_client.size(); i++) {
//             NewClientJoin newClientJoin;
//             newClientJoin.sock = cSock;
//             SendData2All(&newClientJoin);
//         }
        AddClientToCellServer(new ClientSocket(cSock));
        printf("新客户端加入: cSock = %d,客户端数量 = %d, ip = %s \n", (int)cSock, (int)_client.size(), inet_ntoa(clientAddr.sin_addr));
        return cSock;
    }

    void AddClientToCellServer(ClientSocket* pSock) {
        _client.push_back(pSock);

        auto minSer = _cellServer[0];
        for (auto ser : _cellServer)
        {
            if (minSer->GetClientCount() > ser->GetClientCount()) {
                minSer = ser;
            }
        }
        minSer->AddClient(pSock);
    }

    //关闭socket
    void Close() {
        if (INVALID_SOCKET == _sock) {
            return;
        }
#ifdef _WIN32
        //7close-
        for (int i = 0; i < (int)_client.size(); i++) {
            closesocket(_client[i]->GetSocket());
            delete _client[i];
        }
        closesocket(_sock);
        WSACleanup();
#else
        for (int i = 0; i < (int)_client.size(); i++) {
            closesocket(_client[i]->GetSocket());
            delete _client[i];
        }
        close(_sock);
#endif
        _sock = INVALID_SOCKET;
        _client.clear();
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
            int receCount = 0;
            for (auto ser : _cellServer) {
                receCount += ser->_recvCount;
                ser->_recvCount = 0;
            }

            printf("thread<%d> ,time <%lf>, socket <%d>, client <%d>, recvCount<%d>\n", (int)_cellServer.size(), t1,(int)_sock,(int)_client.size(), receCount);
            _tTime.Update();
        }
    }

    //发送数据
    int SendData(DataHeader* hd, SOCKET cSock) {
        if (IsRun() && hd) {
            return send(cSock, (const char*)hd, hd->dataLen, 0);
        }
        return SOCKET_ERROR;
    }

    //对所有连接上的人发送数据
    void SendData2All(DataHeader* hd) {
        if (IsRun() && hd) {
            for (int i = 0; i < (int)_client.size(); i++) {
                SendData(hd, _client[i]->GetSocket());
            }
        }
    }

};


#endif // !_EASY_TCP_SERVER_HPP_
