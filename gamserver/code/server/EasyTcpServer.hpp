#ifndef _EASY_TCP_SERVER_HPP_
#define _EASY_TCP_SERVER_HPP_
#ifdef _WIN32
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
        SOCKET _cSock = INVALID_SOCKET; //��Чsocket
#ifdef _WIN32
        _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
        _cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
        if (INVALID_SOCKET == _cSock) {
            printf("�ͻ�������ʧ�� \n");
        }

        for (int i = 0; i < (int)g_client.size(); i++) {
            NewClientJoin newClientJoin;
            newClientJoin.sock = _cSock;
            SendData2All(&newClientJoin);
        }
        g_client.push_back(_cSock);
        printf("�¿ͻ��˼���: _cSock = %d, ip = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
        return _cSock;
    }

    //�ر�socket
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
        for (int i = 0; i < (int)g_client.size(); i++) {
            FD_SET(g_client[i], &fdRead);
            if (g_client[i] > _sock) {
                maxSock = g_client[i];
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

    //�Ƿ�����
    bool IsRun() {
        return _sock != INVALID_SOCKET;
    }
    //�������� ����ճ�� ��ְ�

    int RecvData(SOCKET _cSock) {
        char szRevc[1024]; //��һ��������
        int nLen = recv(_cSock, szRevc, sizeof(DataHeader), 0);
        DataHeader *hd = (DataHeader*)szRevc;
        if (nLen <= 0) {
            printf("<sock = %d>, �ͻ����Ѿ��˳�\n", _cSock);
            return -1;
        }

        recv(_cSock, szRevc + sizeof(DataHeader), hd->dataLen - sizeof(DataHeader), 0);
        OnNetMsg(hd, _cSock);
        return 0;
    }

    //��Ӧ������Ϣ
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

    //��������
    int SendData(DataHeader* hd, SOCKET _csock) {
        if (IsRun() && hd) {
            return send(_csock, (const char*)hd, hd->dataLen, 0);
        }
        return SOCKET_ERROR;
    }

    //�����������ϵ��˷�������
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
