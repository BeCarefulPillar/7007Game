#include <thread>
#include <atomic>
#include "EasyTcpClient.hpp"
#include "CellTimestame.hpp"

bool g_run = true;
const int cCount = 10;
const int tCount = 4;
EasyTcpClient *client[cCount];
std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;

class MyClient:public EasyTcpClient {
public:
    virtual void OnNetMsg(DataHeader* hd) {
        if (!hd) {
            return;
        }
        switch (hd->cmd) {
            case CMD_LOGIN_RESULT: {
                LoginResult *loginRes = (LoginResult *)hd;
                //CellLog::Info("recv CMD_LOGIN_RESULT dataLen = %d, %s \n", loginRes->dataLen, loginRes->data);
            } break;
            case CMD_LOGOUT_RESULT: {
                LogoutResult *logoutData = (LogoutResult *)hd;
                //CellLog::Info("recv CMD_LOGOUT_RESULT dataLen = %d\n", logoutData->dataLen);
            }break;
            case CMD_NEW_CLIENT_JOIN: {
                NewClientJoin *newClient = (NewClientJoin *)hd;
                //CellLog::Info("recv CMD_NEW_CLIENT_JOIN dataLen = %d sock = %d\n", newClient->dataLen, newClient->sock);
            }break;
            case CMD_ERROR: {
                CellLog::Info("recv CMD_ERROR sock = %d dataLen = %d  \n",_pClient->GetSocket(), hd->dataLen);
            }break;
            default: {
                CellLog::Info("recv 未知消息 sock = %d dataLen = %d  \n", _pClient->GetSocket(), hd->dataLen);
            }
        }
    }
private:

};

void cmdThread(EasyTcpClient * client) {
    while (true) {
        char cmdMsg[32];
        scanf("%s", cmdMsg);
        if (0 == strcmp(cmdMsg, "exit")) {
            CellLog::Info("线程退出 \n");
            g_run = false;
            //client->Close();
            break;
        } else if (0 == strcmp(cmdMsg, "login")) {
            Login loginData;
            strcpy(loginData.account, "ssss");
            strcpy(loginData.password, "123");
            //5 send
            client->SendData(&loginData, loginData.dataLen);
        } else if (0 == strcmp(cmdMsg, "logout")) {
            Logout logoutData;
            strcpy(logoutData.account, "ssss");
            //5 send
            client->SendData(&logoutData, logoutData.dataLen);
        } else {
            CellLog::Info("input error \n");
        }
    }
}

void recvTheard(int begin, int end) {
    CellTimestame t;
    while (g_run) {
        for (int i = begin; i < end; i++) {
//             if (t.GetElapsedSecond() > 3.f&&i == begin) {
//                 continue;
//             }
            client[i]->OnRun();
        }
    }
}

void sendTheard(int id) {
    int begin = (int)(id - 1) * cCount / tCount;
    int end = (int)(id) * cCount / tCount;
    //防止抢占资源
    for (int i = begin; i < end; i++) {
        client[i] = new MyClient();
    }

    for (int i = begin; i < end; i++) {
        client[i]->Connet("127.0.0.1", 4567);
        CellLog::Info("count = %d \n", i);
    }

    readyCount++;
    while (readyCount < tCount)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
   
    std::thread t1(recvTheard, begin, end);
    t1.detach();

    //client.InitSocket();
    const int msgCount = 1;
    Login loginData[msgCount];

    for (int i = 0; i < msgCount; i++) {
        strcpy(loginData[i].account, "ssss");
        strcpy(loginData[i].password, "123");
    }
    int nLen = sizeof(loginData);
    while (g_run) {
        //client.OnRun();
        //test
        for (int i = begin; i < end; i++) {
            if (SOCKET_ERROR != client[i]->SendData(loginData, nLen)) {
                sendCount+=msgCount;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    //---------------------------
    for (int i = begin; i < end; i++) {
        client[i]->Close();
        delete client[i];
    }
}

int main() {

    std::thread cmd(cmdThread, client[0]);
    cmd.detach(); //和主线程分离

    for (int i = 0; i < tCount; i++) {
        std::thread send(sendTheard, i + 1);
        send.detach(); //和主线程分离
        //send.join(); //阻塞主线程，这里可以使用
    }

    CellTimestame _tTime;
    while (g_run) {
        auto t1 = _tTime.GetElapsedSecond();
        if (t1 > 1.0) {
            CellLog::Info("thread<%d> ,time <%lf>, client <%d>, sendCount<%d>, \n", (int)tCount, t1, (int)cCount, (int)sendCount);
            _tTime.Update();
            sendCount = 0;
        }
    }

    CellLog::Info("client out \n");

    getchar();
    return 0;
}
