﻿#include "EasyTcpServer.hpp"
#include <thread>

// bool gRun = true;
// void cmdInput() {
//     while (true) {
//         char cmdMsg[32];
//         scanf("%s", cmdMsg);
//         if (0 == strcmp(cmdMsg, "exit")) {
//             CellLog::Info("线程退出 \n");
//             gRun = false;
//             break;
//         } else {
//             CellLog::Info("input error \n");
//         }
//     }
// }

int main() {
    CellLog::Instance().SetLogPath("serverLog.txt", "w");
    EasyTcpServer server;
    server.InitSocket();
    server.Bind("127.0.0.1", 4567);
    server.Listen(5);
    server.Start();

//     std::thread t1(cmdInput);
//     t1.detach(); //和主线程分离
    bool gRun = true;
    while (gRun) {
        char cmdMsg[32];
        scanf("%s", cmdMsg);
        if (0 == strcmp(cmdMsg, "exit")) {
            CellLog::Info("thread exit \n");
            gRun = false;
            break;
        } else {
            CellLog::Info("input error \n");
        }
        //可以加服务器主动推送
    }
    server.Close();
    Sleep(1000);
    getchar();
    return 0;
}