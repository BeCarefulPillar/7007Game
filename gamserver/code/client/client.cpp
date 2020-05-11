#include <thread>
#include "EasyTcpClient.hpp"

bool g_run = true;
void cmdInput(EasyTcpClient * client) {
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

int main() {
    
    const int count = 100;
    EasyTcpClient *client[count];
    for (int i = 0; i < count; i++) {
        client[i] = new EasyTcpClient();
        client[i]->Connet("127.0.0.1", 4567);
    }
    //client.InitSocket();
    

    std::thread t1(cmdInput, client[0]);
    t1.detach(); //和主线程分离

    Login loginData;
    strcpy(loginData.account, "ssss");
    strcpy(loginData.password, "123");
    while (g_run) {
        //client.OnRun();

        //test
        for (int i = 0; i < count; i++) {
            client[i]->SendData(&loginData);
            //client[i]->OnRun();
        }

        
    }
    //---------------------------
    for (int i = 0; i < count; i++) {
        client[i]->Close();
        delete client[i];
    }
    printf("client out \n");

    getchar();
    return 0;
}
