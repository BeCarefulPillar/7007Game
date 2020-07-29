#include <iostream>
#include "EasyTcpClient.hpp"
//#include "CellStream.hpp"

class MyClient :public EasyTcpClient {
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
    while (client.IsRun()) {
        client.OnRun();
        CellThread::Sleep(10);
    }
    return 0;
}

