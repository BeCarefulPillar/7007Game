#ifndef _CELL_NET_WORK_HPP_
#define _CELL_NET_WORK_HPP_
#include "Cell.hpp"
class CellNetWork {
private:
    CellNetWork() {
#ifdef _WIN32
        WORD ver = MAKEWORD(2, 2); //socket�汾 2.x����
        WSADATA data;
        WSAStartup(ver, &data);
#else
        //�����쳣�źţ�Ĭ������ᵼ�½����˳�
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
