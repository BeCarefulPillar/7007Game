#include <thread>
#include "EasyTcpClient.hpp"

bool g_run = true;
const int cCount = 1000;
const int tCount = 4;
EasyTcpClient *client[cCount];

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
            client->SendData(&loginData);
        } else if (0 == strcmp(cmdMsg, "logout")) {
            Logout logoutData;
            strcpy(logoutData.account, "ssss");
            //5 send
            client->SendData(&logoutData);
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
    //client.InitSocket();

    Login loginData;
    strcpy(loginData.account, "ssss");
    strcpy(loginData.password, "123");
    while (g_run) {
        //client.OnRun();
        //test
        for (int i = begin; i < end; i++) {
            client[i]->SendData(&loginData);
            //client[i]->OnRun();
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
            client[i]->SendData(&loginData);
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
    while (g_run) {
        Sleep(1000);
    }

    printf("client out \n");

    getchar();
    return 0;
}
