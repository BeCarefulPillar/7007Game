#include "EasyTcpServer.hpp"
#include <thread>
#include "CellMsgStream.hpp"

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


class MyServer :public EasyTcpServer {
public:
    virtual void OnNetMsg(CellServer* pCellServer, CellClient* pClient, DataHeader* pHd) {
        _msgCount++;
        if (!pHd) {
            return;
        }
        CellRecvStream r(pHd);
        auto cmd = r.GetNetCmd();
        auto len = r.GetNetLength();

        switch (pHd->cmd) {
        case CMD_LOGIN: {

            auto n1 = r.ReadInt16();
            auto n2 = r.ReadInt32();
            auto n3 = r.ReadFloat();
            auto n4 = r.ReadDouble();
            char un[32] = {};
            r.ReadArray(un, 32);
            char pw[32] = {};
            r.ReadArray(pw, 32);
            int data[10] = {};
            r.ReadArray(data, 10);


            CellSendStream s;
            s.SetNetCmd(CMD_LOGIN_RESULT);
            s.WriteInt16(1);
            s.WriteInt32(2);
            s.WriteFloat(3.0f);
            s.WriteDouble(4.5);

            const char* str = "23333";
            s.WriteArray(str, strlen(str));
            char a[] = "444444";
            s.WriteArray(a, strlen(a));
            int b[] = { 9,9,9,9 };
            s.WriteArray(b, 4);
            s.Finsh();

            //Login *loginData = (Login *)pHd;
            /*CellLog::Info("recv <socket = %d> ,CMD_LOGIN dataLen = %d,account = %s,password=%s \n", pClient->GetSocket(), loginData->dataLen, loginData->account, loginData->password);*/
            //LoginResult loginRes;
            //strcpy(loginRes.data, "ssss");
            if (SOCKET_ERROR == pClient->SendData(s.Data(), s.Length())) {

            }


            //             LoginResult* loginRes =new LoginResult();
            //             pCellServer->AddSendTask(pClient, loginRes);
        } break;
        case CMD_LOGOUT: {
            Logout *logoutData = (Logout *)pHd;
            //CellLog::Info("recv <socket = %d>, CMD_LOGOUT dataLen = %d, account = %s\n", cSock, logoutData->dataLen, logoutData->account);

            //LogoutResult logoutRes;
            //pClient->SendData(&logoutRes);
        }break;
        case CMD_HEART: {
            pClient->ResetDTHeart();
            HeartResult heartRes;
            pClient->SendData(&heartRes);
        }break;
        default: {
            DataHeader head;
            pClient->SendData(&head);
        }break;
        }
    }
private:

};

int main() {
    CellLog::Instance().SetLogPath("serverLog.txt", "w");
    MyServer server;
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
