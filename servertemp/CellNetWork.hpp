#ifndef _CELL_NET_WORK_HPP_
#define _CELL_NET_WORK_HPP_
#include "Cell.hpp"
class CellNetWork {
private:
    CellNetWork() {
#ifdef _WIN32
        WORD ver = MAKEWORD(2, 2); //socket版本 2.x环境
        WSADATA data;
        WSAStartup(ver, &data);
#else
        //忽略异常信号，默认情况会导致进程退出
        signal(SIGPIPE, SIG_IGN);
#endif
    }
    ~CellNetWork() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
public:
    static CellNetWork& Instance() {
        static CellNetWork obj;
        return obj;
    }
private:

};

#endif
