#include <iostream>
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

#include <thread>
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

int process(SOCKET _sock) {
    char szRevc[1024]; //加一个缓冲区
    int nLen = recv(_sock, szRevc, sizeof(dataHeader), 0);
    dataHeader *hd = (dataHeader*)szRevc;
    if (nLen <= 0) {
        printf("server exist out\n");
        return -1;
    }
    switch (hd->cmd) {
    case CMD_LOGIN_RESULT: {
        recv(_sock, szRevc + sizeof(dataHeader), hd->dataLen - sizeof(dataHeader), 0);

        loginResult *loginRes = (loginResult *)szRevc;
        printf("recv CMD_LOGIN_RESULT dataLen = %d, \n", loginRes->dataLen);
    } break;
    case CMD_LOGOUT_RESULT: {
        recv(_sock, szRevc + sizeof(dataHeader), hd->dataLen - sizeof(dataHeader), 0);
        logoutResult *logoutData = (logoutResult *)szRevc;
        printf("recv CMD_LOGOUT_RESULT dataLen = %d\n", logoutData->dataLen);
    }break;
    case CMD_NEW_CLIENT_JOIN: {
        recv(_sock, szRevc + sizeof(dataHeader), hd->dataLen - sizeof(dataHeader), 0);
        newClientJoin *newClient = (newClientJoin *)szRevc;
        printf("recv CMD_NEW_CLIENT_JOIN dataLen = %d sock = %d\n", newClient->dataLen, newClient->sock);
    }break;
    }
    return 0;
}
bool g_bRun = true;
void cmdInput(SOCKET sock) {
    while (true) {
        char cmdMsg[32];
        scanf("%s", cmdMsg);
        if (0 == strcmp(cmdMsg, "exit")) {
            printf("线程退出 \n");
            g_bRun = false;
            break;
        } else if (0 == strcmp(cmdMsg, "login")) {
            login loginData;
            strcpy(loginData.account, "ssss");
            strcpy(loginData.password, "123");
            //5 send
            send(sock, (const char*)&loginData, sizeof(login), 0);
        } else if (0 == strcmp(cmdMsg, "logout")) {
            logout logoutData;
            strcpy(logoutData.account, "ssss");
            //5 send
            send(sock, (const char*)&logoutData, sizeof(logout), 0);
        } else {
            printf("input error \n");
        }
    }
}

int main() {
#ifdef _WIN32
    WORD ver = MAKEWORD(2, 2); //socket版本 2.x环境
    WSADATA data;
    WSAStartup(ver, &data);
#endif
    //---------------------------
    //1 socket
    SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //ipv4, 流数据, tcp
    if (INVALID_SOCKET == _sock) {
        printf("socket error \n");
    } else {
        printf("socket success \n");
    }
    //2 connect
    sockaddr_in _sin = {};
    _sin.sin_family = AF_INET;
    _sin.sin_port = htons(4567);
#ifdef _WIN32
    _sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
#else
    _sin.sin_addr.s_addr = inet_addr("192.168.1.203");
#endif
    if (SOCKET_ERROR == connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in))) {
        printf("connect error \n");
    } else {
        printf("connect succeed \n");
    }
    std::thread t1(cmdInput, _sock);
    t1.detach(); //和主线程分离

    while (g_bRun) {
        fd_set fdRead;
        FD_ZERO(&fdRead);
        FD_SET(_sock, &fdRead);

        timeval t = { 1,0 };
        int ret = select(_sock + 1, &fdRead, 0, 0, &t);
        if (ret < 0) {
            printf("select end \n");
            break;
        }
        if (FD_ISSET(_sock, &fdRead)) {
            FD_CLR(_sock, &fdRead);
            ret = process(_sock);
            if (-1 == ret) {
                printf("client out 1 \n");
            }
        }

        //空闲时发送
    }
    //---------------------------
#ifdef _WIN32
    //7close-
    closesocket(_sock);
    WSACleanup();
#else
    close(_sock);
#endif
    printf("client out 2 \n");
    getchar();
    return 0;
}
