#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN //避免windows.h 和 WinSock2.h 中的宏定义重复
#include <windows.h>
#include <WinSock2.h>
#else
#include <unistd.h> //uni std
#include <arpa/inet.h>
#include <string.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif
#include <stdio.h>
#include <thread>
#include <vector>
//静态链接库 win平台
//#pragma comment(lib, "ws2_32.lib")

enum CMD {
    CMD_ERROR = 0,
    CMD_LOGIN,
    CMD_LOGIN_RESULT,
    CMD_LOGOUT,
    CMD_LOGOUT_RESULT,
    CMD_NEW_CLIENT_JOIN,
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

struct newClientJoin : public dataHeader {
    newClientJoin() {
        cmd = CMD_NEW_CLIENT_JOIN;
        dataLen = sizeof(logoutResult);
    };
    int sock;
};

int process(SOCKET _cSock) {
    char szRevc[1024]; //加一个缓冲区
    int nLen = recv(_cSock, szRevc, sizeof(dataHeader), 0);
    dataHeader *hd = (dataHeader*)szRevc;
    if (nLen <= 0) {
        printf("sock = %d, client exist out\n", _cSock);
        return -1;
    }
    switch (hd->cmd) {
    case CMD_LOGIN: {
        recv(_cSock, szRevc + sizeof(dataHeader), hd->dataLen - sizeof(dataHeader), 0);

        login *loginData = (login *)szRevc;
        printf("recv <socket = %d> ,CMD_LOGIN dataLen = %d,account = %s,password=%s \n", _cSock, loginData->dataLen, loginData->account, loginData->password);

        loginResult loginRes;
        send(_cSock, (const char *)&loginRes, sizeof(loginResult), 0);
    } break;
    case CMD_LOGOUT: {
        recv(_cSock, szRevc + sizeof(dataHeader), hd->dataLen - sizeof(dataHeader), 0);
        logout *logoutData = (logout *)szRevc;
        printf("recv <socket = %d>, CMD_LOGOUT dataLen = %d, account = %s\n", _cSock, logoutData->dataLen, logoutData->account);

        logoutResult logoutRes;
        send(_cSock, (const char *)&logoutRes, sizeof(logoutResult), 0);
    }break;
    default: {
        dataHeader head = { CMD_ERROR , 0 };
        send(_cSock, (const char *)&head, sizeof(dataHeader), 0);
    }break;
    }
    return 0;
}

std::vector<SOCKET> g_client;
int main() {
#ifdef _WIN32
    WORD ver = MAKEWORD(2, 2); //socket版本 2.x环境
    WSADATA data;
    WSAStartup(ver, &data);
#endif
    //---------------------------
    //-socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //ipv4, 流数据, tcp
    if (INVALID_SOCKET == _sock) {
        printf("socket error \n");
    } else {
        printf("socket success \n");
    }
    //-bind-
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567); //主机数据转换到网络数据 host to net unsigned short
#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = inet_addr("192.168.1.203"); //INADDR_ANY; // ip
#else
    _sin.sin_addr.s_addr = inet_addr("192.168.1.181"); //INADDR_ANY; // ip
#endif
    if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in))) {
        printf("bind error \n");
    } else {
        printf("bind success \n");
    }
    //-listen-
    if (SOCKET_ERROR == listen(_sock, 5)) {//监听人数 
        printf("listen error \n");
    } else {
        printf("listen success \n");
    }

    while (true) {
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
            printf("select end \n");
            break;
        }
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);
            //-accept-
            sockaddr_in clientAddr = {};
            int nAddrLen = sizeof(sockaddr_in);
            SOCKET _cSock = INVALID_SOCKET; //无效socket
#ifdef _WIN32
            _cSock = accept(_sock, (sockaddr*)&clientAddr, &nAddrLen);
#else
            _cSock = accept(_sock, (sockaddr*)&clientAddr, (socklen_t*)&nAddrLen);
#endif
            if (INVALID_SOCKET == _cSock) {
                printf("error, client invalid socket \n");
            }

            for (int i = 0; i < (int)g_client.size(); i++) {
                newClientJoin newClientJoin;
                newClientJoin.sock = _cSock;
                send(g_client[i], (const char *)&newClientJoin, sizeof(logoutResult), 0);
            }
            g_client.push_back(_cSock);
            printf("new client add: _cSock = %d, ip = %s \n", (int)_cSock, inet_ntoa(clientAddr.sin_addr));
        }

        for (int i = 0; i < (int)g_client.size(); i++) {
            if (FD_ISSET(g_client[i], &fdRead)) {
                int ret = process(g_client[i]);
                if (ret == -1) {
                    auto iter = g_client.begin() + i;
                    if (iter != g_client.end()) {
                        g_client.erase(iter);
                    }
                }
            }
        }
        //可以加服务器主动推送
    }
#ifdef _WIN32
    //-close-
    closesocket(_sock);
    //---------------------------
    WSACleanup();
#else
    close(_sock);
#endif
    printf("server exist out\n");
    getchar();
    return 0;
}