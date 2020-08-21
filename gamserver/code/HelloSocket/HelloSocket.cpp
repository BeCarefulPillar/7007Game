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

        switch (hd->cmd) {
        case CMD_LOGIN_RESULT: {
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
int main()
{
    MyClient client;
    client.Connet("127.0.0.1", 4567);
    CellSendStream s;
    s.SetNetCmd(CMD_LOGIN);
    s.WriteInt16(1);
    s.WriteInt32(2);
    s.WriteFloat(3.0f);
    s.WriteDouble(4.5);

    const char* str = "what";
    s.WriteArray(str, strlen(str));
    char a[] = "eiei";
    s.WriteArray(a, strlen(a));
    int b[] = {1,2,3,4,5};
    s.WriteArray(b, 5);
    s.Finsh();


    CellRecvStream r(s.Data(), s.Length());
    auto cmd = r.GetNetCmd();
    auto len = r.GetNetLength();
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

    client.SendData(s.Data(), s.Length());
    while (client.IsRun()) {
        client.OnRun();
        CellThread::Sleep(10);
    }
    return 0;
}

