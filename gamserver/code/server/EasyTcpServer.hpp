#ifndef _EASY_TCP_SERVER_HPP_
#define _EASY_TCP_SERVER_HPP_
#ifdef _WIN32
#define FD_SETSIZE 1024
#define WIN32_LEAN_AND_MEAN //避免windows.h 和 WinSock2.h 中的宏定义重复

#include <windows.h>
#include <WinSock2.h>
#include <memory> //智能指针
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

#ifndef SEND_BUFF_SIZE
#define SEND_BUFF_SIZE 10240
#endif // !SEND_BUFF_SIZE

#ifndef CELL_SERVER_THEARD
#define CELL_SERVER_THEARD 4
#endif

#include <stdio.h>
#include <vector>
#include <thread>
#include <mutex> //锁
#include <atomic>
#include <map>
#include "MessageHeader.hpp"
#include "CellTimestame.hpp"
#include "CellTask.hpp"
//客户端数据类型

typedef std::shared_ptr<DataHeader> DataHeaderPtr;

class ClientSocket {
private:
    SOCKET _sock;
    char _szMsgBuf[REVC_BUFF_SIZE]; //第二缓冲区，消息缓冲区
    int _lastPos;//消息缓冲区结尾
    sockaddr_in _addr;
    int _lastSendPos;//消息缓冲区结尾
    char _szSendBuf[SEND_BUFF_SIZE];
public:
    ClientSocket(SOCKET sock, sockaddr_in addr) {
        _sock = sock;
        _lastPos = 0;
        _lastSendPos = 0;
        memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
        memset(_szSendBuf, 0, sizeof(_szSendBuf));
        _addr = addr;
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

    int SendData(DataHeaderPtr& pHd) {
        int ret = SOCKET_ERROR;
        if (!pHd) {
            return ret;
        }
        
        int nSendLen = pHd->dataLen;
        const char* pSendData = (const char*)pHd.get();
        while (true) {
            if (_lastSendPos + nSendLen >= SEND_BUFF_SIZE) {
                int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
                memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
                //计算剩余数据位置
                pSendData += nCopyLen;
                //计算剩余长度
                nSendLen -= nCopyLen;
                //send
                ret = send(_sock, _szSendBuf, SEND_BUFF_SIZE, 0);
                _lastSendPos = 0;
                if (SOCKET_ERROR == ret) {
                    return ret;
                }
            } else {
                memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
                _lastSendPos += nSendLen;
                break;
            }
        }
        return ret;
    }
};

typedef std::shared_ptr<ClientSocket> ClientSocketPtr;
class CellServer;
//网络事件接口
class INetEvent {
public:
    virtual void OnNetJoin(ClientSocketPtr& pClient) = 0;
    virtual void OnNetLevel(ClientSocketPtr& pClient) = 0;
    virtual void OnNetMsg(CellServer* pCellServer, ClientSocketPtr& pClient, DataHeader* pHd) = 0;
    virtual void OnNetRecv(ClientSocketPtr& pClient) = 0;
};

class CellS2CTask : public CellTask{
    ClientSocketPtr _pClient;
    DataHeaderPtr _pHeader;
public:
    CellS2CTask(ClientSocketPtr& pClient, DataHeaderPtr& pHd) {
        _pClient = pClient;
        _pHeader = pHd;
    }
    ~CellS2CTask() {

    }

    virtual void DoTask() {
        _pClient->SendData(_pHeader);
    }

private:

};

class CellServer {
private:
    SOCKET _sock;
    //正式
    std::map<SOCKET, ClientSocketPtr> _client;
    //缓冲
    std::vector<ClientSocketPtr> _clientBuff;
    std::mutex _mutex;
    INetEvent* _pNetEvent;
    CellTaskServer _taskServer;
    fd_set _fdReadBak;
    bool _clientChange;
    SOCKET _maxSocket;
public:
    CellServer(SOCKET socket) {
        _sock = socket;
        _pNetEvent = nullptr;
        _clientChange = false;
    }

    ~CellServer() {
        _taskServer.Stop();
        Close();
    }
    void AddSendTask(ClientSocketPtr& pClient, DataHeaderPtr& pHd) {
        auto task = std::make_shared<CellS2CTask>(pClient, pHd);
        _taskServer.AddTask((std::shared_ptr<CellTask>&)task);
    }

    void SetNetObj(INetEvent* event) {
        _pNetEvent = event;
    }

    int GetClientCount() {
        return _clientBuff.size() + _client.size();
    }

    void AddClient(ClientSocketPtr& pClient) {
        std::lock_guard<std::mutex> lock(_mutex);
        _clientBuff.push_back(pClient); //在线程中要使用，所以要加锁
    }

    void Start() {
        std::thread _pThread(std::mem_fn(&CellServer::OnRun), this); //成员函数转函数对象，使用对象指针或引用进行绑定
        _pThread.detach();
        _taskServer.Start();
    }

    //关闭socket
    void Close() {
        if (INVALID_SOCKET == _sock) {
            return;
        }
#ifdef _WIN32
        for (auto c : _client) {
            closesocket(c.second->GetSocket());
        }
        //closesocket(_sock);
#else
        for (auto c : _client) {
            close(c.second->GetSocket());
        }
        //close(_sock);
#endif
        _client.clear();
        _sock = INVALID_SOCKET;
    }

    bool OnRun() {
        while (IsRun()) {
            auto a = IsRun();
            if (!_clientBuff.empty()){
                std::lock_guard<std::mutex> lock(_mutex);
                for (auto pClient : _clientBuff){
                    _client[pClient->GetSocket()] = pClient;
                }
                _clientBuff.clear();
                _clientChange = true;
            }

            if (_client.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            fd_set fdRead;
            FD_ZERO(&fdRead);

            if (_clientChange) {
                _clientChange = false;
                _maxSocket = _client.begin()->second->GetSocket();
                for (auto c : _client)
                {
                    FD_SET(c.first, &fdRead);
                    if (c.first > _maxSocket) {
                        _maxSocket = c.first;
                    }                }
                memcpy(&_fdReadBak, &fdRead, sizeof(fd_set));
            } else {
                memcpy(&fdRead, &_fdReadBak, sizeof(fd_set));
            }

            int ret = select(_maxSocket + 1, &fdRead, nullptr, nullptr, nullptr); //select 性能瓶颈 最大的集合只有64
            if (ret < 0) {
                printf("select 结束 \n");
                Close();
                return false;
            } else if (ret == 0) {
                continue;
            }
#ifdef WIN32
            for (int i = 0; i < (int)fdRead.fd_count; i++) {
                auto iter = _client.find(fdRead.fd_array[i]);
                if (iter != _client.end()) {
                    int ret = RecvData(iter->second);
                    if (ret == -1) {
                        if (_pNetEvent) {
                            _pNetEvent->OnNetLevel(iter->second);
                        }
                        _client.erase(iter->first);
                        _clientChange = true;
                    }
                } else {
                    printf("error socket \n");
                }
            }

#else
            std::vector <ClientSocketPtr> temp;
            for (auto c : _client) {
                if (FD_ISSET(c.first, &fdRead)) {
                    int ret = RecvData(c.second);
                    if (ret == -1) {
                        if (_pNetEvent) {
                            _pNetEvent->OnNetLevel(c.second);
                        }
                        temp.push_back(c.second);
                        _clientChange = true;
                    }
                }
            }

            for (auto client : temp) {
                _client.erase(client->GetSocket());
            }
#endif // WIN32
            
        }
        return true;
    }
    //是否工作中
    bool IsRun() {
        return _sock != INVALID_SOCKET;
    }

    //接收数据 处理粘包 拆分包
    int RecvData(ClientSocketPtr& pClient) {
        char* szRevc = pClient->MsgBuf() + pClient->GetLastPos();
        int nLen = recv(pClient->GetSocket(), szRevc, REVC_BUFF_SIZE - pClient->GetLastPos(), 0);
        if (nLen <= 0) {
            return -1;
        }
        _pNetEvent->OnNetRecv(pClient);
        //消息缓冲区数据尾部向后
        pClient->SetLastPos(pClient->GetLastPos() + nLen);
        //判断消息长度大于消息头
        while (pClient->GetLastPos() >= sizeof(DataHeader)) {
            //当前消息
            DataHeader *hd = (DataHeader*)pClient->MsgBuf();
            if (pClient->GetLastPos() >= hd->dataLen) {
                int msgLen = hd->dataLen;
                //处理消息
                OnNetMsg(pClient, hd);
                //数据前移
                memcpy(pClient->MsgBuf(), pClient->MsgBuf() + msgLen, pClient->GetLastPos() - msgLen);
                //消息尾部前移
                pClient->SetLastPos(pClient->GetLastPos() - msgLen);
            } else {
                break;
            }
        }

        return 0;
    }

    virtual void OnNetMsg(ClientSocketPtr& pClient, DataHeader* pHd) {
        if (pClient && pHd) {
            _pNetEvent->OnNetMsg(this, pClient, pHd);
        }
    }

};

typedef std::shared_ptr<CellServer> CellServerPtr;
class EasyTcpServer: public INetEvent {
private:
    SOCKET _sock;
    std::atomic_int _clientCount; 
    std::vector<CellServerPtr> _cellServer;//使用指针的原因是栈空间只有1M到2M
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
            auto server = std::make_shared<CellServer>(_sock);
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
        ClientSocketPtr pClient = std::make_shared<ClientSocket>(cSock, clientAddr);
        AddClientToCellServer(pClient);
        //printf("新客户端加入: cSock = %d,客户端数量 = %d, ip = %s \n", (int)cSock, _clientCount, inet_ntoa(clientAddr.sin_addr));
        return cSock;
    }

    void AddClientToCellServer(ClientSocketPtr& pClient) {
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

    virtual void OnNetJoin(ClientSocketPtr& pClient) {
        _clientCount++;
    }

    virtual void OnNetLevel(ClientSocketPtr& pClient) {
        _clientCount--;
    }

    virtual void OnNetRecv(ClientSocketPtr& pClient) {
        _recvCount++;
    }

    virtual void OnNetMsg(CellServer* pCellServer , ClientSocketPtr& pClient, DataHeader* pHd) {
        _msgCount++;
        if (!pHd) {
            return;
        }
        switch (pHd->cmd) {
        case CMD_LOGIN: {
            Login *loginData = (Login *)pHd;
            /*printf("recv <socket = %d> ,CMD_LOGIN dataLen = %d,account = %s,password=%s \n", pClient->GetSocket(), loginData->dataLen, loginData->account, loginData->password);*/

            auto loginRes = std::make_shared<LoginResult>(
                );
            pCellServer->AddSendTask(pClient, (DataHeaderPtr&)loginRes);
        } break;
        case CMD_LOGOUT: {
            Logout *logoutData = (Logout *)pHd;
            //printf("recv <socket = %d>, CMD_LOGOUT dataLen = %d, account = %s\n", cSock, logoutData->dataLen, logoutData->account);

            //LogoutResult logoutRes;

            //pClient->SendData(&logoutRes);
        }break;
        default: {
            auto pHead = std::make_shared<DataHeader>(
                );
            pClient->SendData(pHead);
        }break;
        }
    }

};


#endif // !_EASY_TCP_SERVER_HPP_
