#ifndef _EASY_TCP_CLIENT_HPP_
#define _EASY_TCP_CLIENT_HPP_

#include "Cell.hpp"
#include "CellNetWork.hpp"
#include "MessageHeader.hpp"
#include "CellClient.hpp"

class EasyTcpClient {
protected:
    CellClient* _pClient = nullptr;;
    bool _isConnect = false;
public:
    EasyTcpClient() {
        _isConnect = false;
    }
    virtual ~EasyTcpClient() {
        Close();
    }
    //初始化socket
    void InitSocket() {
        CellNetWork::Instance();

        if (_pClient) {
            CellLog::Info("<socket = %d>close old connct \n", _pClient->GetSocket());
            Close();
        }
        SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //ipv4, 流数据, tcp
        if (INVALID_SOCKET == sock) {
            CellLog::Info("socket error \n");
        } else {
            _pClient = new CellClient(sock);
        }
    }
    //连接服务器
    int Connet(const char* ip, unsigned short port) {
        if (!_pClient) {
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
        if (SOCKET_ERROR == connect(_pClient->GetSocket(), (sockaddr*)&_sin, sizeof(sockaddr_in))) {
            CellLog::Info("connect error \n");
            Close();
            return -1;
        } else {
            _isConnect = true;
            //CellLog::Info("<sock = %d> 连接 <%s:%d>成功 \n", (int)_pClient->GetSocket(), ip, port);
            return 0;
        }
    }
    //关闭连接
    void Close() {
        if (_pClient) {
            delete  _pClient;
            _pClient = nullptr;
        }
        _isConnect = false;
    }
    //运行select
    bool OnRun() {
        if (!IsRun()) {
            return false;
        }

        SOCKET sock = _pClient->GetSocket();

        fd_set fdRead;
        FD_ZERO(&fdRead);
        FD_SET(sock, &fdRead);

        fd_set fdWrite;
        FD_ZERO(&fdWrite);

        timeval t = { 0, 10 };
        int ret = 0;
        if (_pClient->NeedWrite()) {
            FD_SET(sock, &fdWrite);
            ret = select((int)sock + 1, &fdRead, &fdWrite, nullptr, &t);
        } else {
            ret = select((int)sock + 1, &fdRead, nullptr, nullptr, &t);
        }

        if (ret < 0) {
            CellLog::Info("客户端关闭1 \n");
            Close();
            return false;
        }
        if (FD_ISSET(sock, &fdRead)) {
            if (-1 == RecvData()) {
                CellLog::Info("客户端关闭2 \n");
                Close();
                return false;
            }
        }
        if (FD_ISSET(sock, &fdWrite)) {
            if (-1 == _pClient->SendDataReal()) {
                CellLog::Info("客户端关闭2 \n");
                Close();
                return false;
            }
        }
        return true;
    }

    bool IsRun() {
        return _pClient && _isConnect;
    }

    char _szMsgBuf[REVC_BUFF_SIZE] = {}; //第二缓冲区，消息缓冲区
    int _lastPos = 0;//消息缓冲区结尾
    //处理粘包，拆包
    int RecvData() {
        int nLen = _pClient->RecvData();
        if (nLen > 0) {
            //判断完整消息
            while (_pClient->HasMsg()) {
                //处理消息
                OnNetMsg(_pClient->FrontMsg());
                //数据前移
                //移除消息队列（缓冲区）最前的一条数据
                _pClient->PopFrontMsg();
            }
        }
        return nLen;
    }
    //响应网络消息
    virtual void OnNetMsg(DataHeader* hd) = 0;

    int SendData(DataHeader* hd, int nLen) {
        if (!_pClient){
            return 0;
        }
        return _pClient->SendData(hd);
    }

private:

};

#endif