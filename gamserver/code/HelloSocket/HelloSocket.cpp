#include <iostream>
#include "EasyTcpClient.hpp"
#include "CellMsgStream.hpp"

class MyClient :public EasyTcpClient {
public:
    virtual void OnNetMsg(DataHeader* hd) {
        if (!hd) {
            return;
        }

        CellRecvStream r(hd);
        auto cmd = r.GetNetCmd();
        auto len = r.GetNetLength();
        CellLog::Info("recv CMD_LOGIN_RESULT cmd = %d, len = %d \n", cmd, len);
        switch (hd->cmd) {
        case CMD_LOGIN_RESULT: {
            //auto n1 = r.ReadInt16();
            //auto n2 = r.ReadInt32();
            ////auto n3 = r.ReadFloat();
            ////auto n4 = r.ReadDouble();
            //char un[32] = {};
            //r.ReadArray(un, 32);
            //char pw[32] = {};
            //r.ReadArray(pw, 32);
            //char data[10] = {};
            //r.ReadArray(data, 10);

            
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
            CellLog::Info("recv CMD_ERROR sock = %d dataLen = %d  \n", _pClient->GetSocket(), hd->dataLen);
        }break;
        default: {
            CellLog::Info("recv 未知消息 sock = %d dataLen = %d  \n", _pClient->GetSocket(), hd->dataLen);
        }
        }
    }
private:

};

void getStr(char * r) {
}

int main()
{
    const char* abc = "1111111111";
    void *d = &abc;

    int zzz = 10;
    char* c = (char*) &zzz;

    MyClient client;
    //client.Connet("127.0.0.1", 4567);

    client.Connet("192.168.1.181", 4500);
    CellSendStream s;
    s.SetNetCmd(555);

    const char* str = "what";
    s.WriteArray(str, strlen(str));
    char a[] = "eiei";
    s.WriteArray(a, strlen(a));
    const char* b = "222222222";
    s.WriteArray(b, strlen(b));
    s.Finsh();

    while (client.IsRun()) {
        client.SendData(s.Data(), s.Length());
        client.OnRun();
        CellThread::Sleep(100);
    }
    return 0;
}

