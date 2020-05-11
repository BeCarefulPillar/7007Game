#ifndef _EASY_TCP_SERVER_HPP_
#define _EASY_TCP_SERVER_HPP_
#ifdef _WIN32
#define FD_SETSIZE 1024
#define WIN32_LEAN_AND_MEAN //����windows.h �� WinSock2.h �еĺ궨���ظ�

#include <windows.h>
#include <WinSock2.h>
//��̬���ӿ� winƽ̨
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

#include <stdio.h>
#include <vector>
#include "MessageHeader.hpp"
#include "CellTimestame.hpp"

class ClientSocket {
private:
    SOCKET _sock;
    char _szMsgBuf[REVC_BUFF_SIZE * 10]; //�ڶ�����������Ϣ������
    int _lastPos;//��Ϣ��������β
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

class EasyTcpServer {
    SOCKET _sock;
    std::vector<ClientSocket *> _client; //ʹ��ָ���ԭ����ջ�ռ�ֻ��1M��2M
    CellTimestame _tTime;
    int _recvCount;
public:
    EasyTcpServer() {
        _sock = INVALID_SOCKET;
        _recvCount = 0;
    }
    virtual ~EasyTcpServer() {
        Close();
    }

    //��ʼ��socket
    void InitSocket() {
        if (INVALID_SOCKET != _sock) {
            Close();
        }
#ifdef _WIN32
        WORD ver = MAKEWORD(2, 2); //socket�汾 2.x����
        WSADATA data;
        WSAStartup(ver, &data);
#endif
        //---------------------------
        //-socket
        _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //ipv4, ������, tcp
        if (INVALID_SOCKET == _sock) {
            printf("socket��ʼ������ \n");
        } else {
            printf("socket��ʼ���ɹ� \n");
        }
    }
    //�󶨶˿ں�
    int Bind(const char * ip, unsigned short port) {
        sockaddr_in _sin = {};
        _sin.sin_family = AF_INET;
        _sin.sin_port = htons(port); //��������ת������������ host to net unsigned short
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
            printf("���Ӷ˿�<port = %d>���� \n",port);
            Close();
        } else {
            printf("���Ӷ˿�<port = %d> �ɹ� \n",port);
        }
        return ret;
    }
    //�����˿ں�
    int Listen(int n) {
        int ret = listen(_sock, n);
        if (SOCKET_ERROR == ret) {//�������� 
            printf("�������� \n");
            Close();
        } else {
            printf("�����ɹ� \n");
        }
        return ret;
    }
    //���ܿͻ�������
    SOCKET Accept(){
        sockaddr_in clientAddr = {};
        int nAddrLen = sizeof(sockaddr_in);
        SOCKET cSock = INVALID_SOCKET; //��Чsocket
#ifdef _WIN32
        cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
        cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
        if (INVALID_SOCKET == cSock) {
            printf("�ͻ�������ʧ�� \n");
        }

        for (int i = 0; i < (int)_client.size(); i++) {
            NewClientJoin newClientJoin;
            newClientJoin.sock = cSock;
            SendData2All(&newClientJoin);
        }
        _client.push_back(new ClientSocket(cSock));
        printf("�¿ͻ��˼���: cSock = %d,�ͻ������� = %d, ip = %s \n", (int)cSock, (int)_client.size(), inet_ntoa(clientAddr.sin_addr));
        return cSock;
    }

    //�ر�socket
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
    //����������Ϣ
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
        //������ socket
        /*param
        1.��windows��û������, fd_set ���м����� ������(socket)��Χ, ����������
        2.�ɶ�����
        3.��д����
        4.�쳣����
        5.��ʱʱ��
        */
        int maxSock = _sock;
        for (int i = 0; i < (int)_client.size(); i++) {
            FD_SET(_client[i]->GetSocket(), &fdRead);
            if (_client[i]->GetSocket() > _sock) {
                maxSock = _client[i]->GetSocket();
            }
        }

        timeval time = { 1, 0 };//�����������Ϊnull ���Ե���һ������Ҫ�ͻ�������ķ�����
        int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExcept, &time); //select ����ƿ�� ���ļ���ֻ��64
        if (ret < 0) {
            printf("select ���� \n");
            Close();
            return false;
        }
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);
            //-accept-
            Accept();
        }
        for (int i = 0; i < (int)_client.size(); i++) {
            if (FD_ISSET(_client[i]->GetSocket(), &fdRead)) {
                int ret = RecvData(_client[i]);
                if (ret == -1) {
                    auto iter = _client.begin() + i;
                    if (iter != _client.end()) {
                        delete _client[i];
                        _client.erase(iter);
                    }
                }
            }
        }
        return true;
    }

    //�Ƿ�����
    bool IsRun() {
        return _sock != INVALID_SOCKET;
    }
    //�������� ����ճ�� ��ְ�

    char _szRevc[REVC_BUFF_SIZE]; //��һ��������
    int RecvData(ClientSocket* client) {
        _recvCount++;
        auto t1 = _tTime.GetElapsedSecond();
        if (t1 > 1.0) {
            printf("<sock = %d>, �ͻ������� = %d, time = %f ,recvCount = %d\n", (int)_sock, (int)_client.size(), t1, _recvCount);
            _recvCount = 0;
            _tTime.Update();
        }
        int nLen = recv(client->GetSocket(), _szRevc, REVC_BUFF_SIZE, 0);
        if (nLen <= 0) {
            printf("<sock = %d>, �ͻ����Ѿ��˳�\n", client->GetSocket());
            return -1;
        }

        //�յ����ݼ��뻺����
        memcpy(client->MsgBuf() + client->GetLastPos(), _szRevc, nLen);
        //��Ϣ����������β�����
        client->SetLastPos(client->GetLastPos() + nLen);
        //�ж���Ϣ���ȴ�����Ϣͷ
        while (client->GetLastPos() >= sizeof(DataHeader)) {
            //��ǰ��Ϣ
            DataHeader *hd = (DataHeader*)client->MsgBuf();
            if (client->GetLastPos() >= hd->dataLen) {
                int msgLen = hd->dataLen;
                //������Ϣ
                OnNetMsg(hd, client->GetSocket());
                //����ǰ��
                memcpy(client->MsgBuf(), client->MsgBuf() + msgLen, client->GetLastPos() - msgLen);
                //��Ϣβ��ǰ��
                client->SetLastPos(client->GetLastPos() - msgLen);
            } else {
                break;
            }
        }

        return 0;
    }

    //��Ӧ������Ϣ
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

    //��������
    int SendData(DataHeader* hd, SOCKET cSock) {
        if (IsRun() && hd) {
            return send(cSock, (const char*)hd, hd->dataLen, 0);
        }
        return SOCKET_ERROR;
    }

    //�����������ϵ��˷�������
    void SendData2All(DataHeader* hd) {
        if (IsRun() && hd) {
            for (int i = 0; i < (int)_client.size(); i++) {
                SendData(hd, _client[i]->GetSocket());
            }
        }
    }


private:

};


#endif // !_EASY_TCP_SERVER_HPP_
