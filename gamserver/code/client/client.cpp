#include <thread>
#include <atomic>
#include "EasyTcpClient.hpp"
#include "CellTimestame.hpp"

bool g_run = true;
const int cCount = 1000;
const int tCount = 4;
EasyTcpClient *client[cCount];
std::atomic_int sendCount = 0;
std::atomic_int readyCount = 0;

void cmdThread(EasyTcpClient * client) {
    while (true) {
        char cmdMsg[32];
        scanf("%s", cmdMsg);
        if (0 == strcmp(cmdMsg, "exit")) {
            printf("线程退出 \n");
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
            printf("input error \n");
        }
    }
}

void sendTheard(int id) {
    int begin = (int)(id - 1) * cCount / tCount;
    int end = (int)(id) * cCount / tCount;
    //防止抢占资源
    for (int i = begin; i < end; i++) {
        client[i] = new EasyTcpClient();
    }

    for (int i = begin; i < end; i++) {
        client[i]->Connet("127.0.0.1", 4567);
        printf("count = %d \n", i);
    }

    readyCount++;
    while (readyCount < tCount)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    //client.InitSocket();

    Login loginData[10];
    for (int i = 0; i < 10; i++) {
        strcpy(loginData[i].account, "ssss");
        strcpy(loginData[i].password, "123");
    }
    int nLen = sizeof(loginData);
    while (g_run) {
        //client.OnRun();
        //test
        for (int i = begin; i < end; i++) {
            if (SOCKET_ERROR != client[i]->SendData(loginData, nLen)) {
                sendCount+=10;
            }
            
            client[i]->OnRun();
        }
    }
    //---------------------------
    for (int i = begin; i < end; i++) {
        client[i]->Close();
        delete client[i];
    }
}

void mainSend() {
    for (int i = 0; i < cCount; i++) {
        client[i] = new EasyTcpClient();
    }

    for (int i = 0; i < cCount; i++) {
        client[i]->Connet("127.0.0.1", 4567);
        printf("count = %d \n", i);
    }
    //client.InitSocket();

    Login loginData;
    strcpy(loginData.account, "ssss");
    strcpy(loginData.password, "123");
    while (g_run) {
        //client.OnRun();
        //test
        for (int i = 0; i < cCount; i++) {
            client[i]->SendData(&loginData, loginData.dataLen);
            //client[i]->OnRun();
        }
    }
    //---------------------------
    for (int i = 0; i < cCount; i++) {
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


   // mainSend();
    CellTimestame _tTime;
    while (g_run) {
        auto t1 = _tTime.GetElapsedSecond();
        if (t1 > 1.0) {
            printf("thread<%d> ,time <%lf>, client <%d>, sendCount<%d>, \n", (int)tCount, t1, (int)cCount, (int)sendCount);
            _tTime.Update();
            sendCount = 0;
        }
        Sleep(1000);
    }

    printf("client out \n");

    getchar();
    return 0;
}
