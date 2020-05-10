#include <thread>
#include "EasyTcpClient.hpp"

void cmdInput(EasyTcpClient *client) {
    while (true) {
        char cmdMsg[32];
        scanf("%s", cmdMsg);
        if (0 == strcmp(cmdMsg, "exit")) {
            printf("线程退出 \n");
            client->Close();
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
    
    EasyTcpClient client;
    //client.InitSocket();
    client.Connet("127.0.0.1", 4567);

    std::thread t1(cmdInput, &client);
    t1.detach(); //和主线程分离

    Login loginData;
    strcpy(loginData.account, "ssss");
    strcpy(loginData.password, "123");
    while (client.IsRun()) {
        client.OnRun();

        //test
        //client.SendData(&loginData);
        
    }
    //---------------------------
    client.Close();
    printf("client out \n");

    getchar();
    return 0;
}
