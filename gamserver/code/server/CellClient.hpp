#ifndef _CELL_CLIENT_HPP
#define _CELL_CLIENT_HPP
#include "Cell.hpp"
//客户端数据类型
class CellClient {
private:
    SOCKET _sock;
    char _szMsgBuf[REVC_BUFF_SIZE]; //第二缓冲区，消息缓冲区
    int _lastPos;//消息缓冲区结尾
    sockaddr_in _addr;
    int _lastSendPos;//消息缓冲区结尾
    char _szSendBuf[SEND_BUFF_SIZE];
public:
    CellClient(SOCKET sock, sockaddr_in addr) {
        _sock = sock;
        _lastPos = 0;
        _lastSendPos = 0;
        memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
        memset(_szSendBuf, 0, sizeof(_szSendBuf));
        _addr = addr;
    }

    SOCKET GetSocket() {
        return _sock;
    }

    char* MsgBuf() {
        return _szMsgBuf;
    }

    int GetLastPos() {
        return _lastPos;
    }

    void SetLastPos(int lastPos) {
        _lastPos = lastPos;
    }

    int SendData(DataHeader* hd) {
        int ret = SOCKET_ERROR;
        if (!hd) {
            return ret;
        }

        int nSendLen = hd->dataLen;
        const char* pSendData = (const char*)hd;
        while (true) {
            if (_lastSendPos + nSendLen >= SEND_BUFF_SIZE) {
                int nCopyLen = SEND_BUFF_SIZE - _lastSendPos;
                memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
                //计算剩余数据位置
                pSendData += nCopyLen;
                //计算剩余长度
                nSendLen -= nCopyLen;
                //send
                ret = send(_sock, _szSendBuf, SEND_BUFF_SIZE, 0);
                _lastSendPos = 0;
                if (SOCKET_ERROR == ret) {
                    return ret;
                }
            } else {
                memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
                _lastSendPos += nSendLen;
                break;
            }
        }
        return ret;
    }
};
#endif
