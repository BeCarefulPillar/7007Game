#ifndef _CELL_HPP
#define _CELL_HPP

#ifdef _WIN32
#define FD_SETSIZE 1024
#define WIN32_LEAN_AND_MEAN //避免windows.h 和 WinSock2.h 中的宏定义重复
#include <windows.h>
#include <WinSock2.h>
//静态链接库 win平台
//#pragma comment(lib, "ws2_32.lib")
#else
#include <unistd.h> //uni std
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#endif

#ifndef REVC_BUFF_SIZE
#define REVC_BUFF_SIZE 10240 * 5
#endif // !REVC_BUFF_SIZE

#ifndef SEND_BUFF_SIZE
#define SEND_BUFF_SIZE 10240
#endif // !SEND_BUFF_SIZE

#ifndef CELL_SERVER_THEARD
#define CELL_SERVER_THEARD 4
#endif

#include "MessageHeader.hpp"
#include "CellTimestame.hpp"
#include "CellTask.hpp"
#include "CellLog.hpp"
#endif
