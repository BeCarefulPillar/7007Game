#include "EasyTcpServer.hpp"

int main() {
    EasyTcpServer server;
    server.InitSocket();
    server.Bind("127.0.0.1", 4567);
    server.Listen(5);

    while (server.IsRun()) {
        server.OnRun();
        //可以加服务器主动推送
    }
    server.Close();
    getchar();
    return 0;
}
