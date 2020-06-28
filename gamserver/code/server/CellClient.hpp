#ifndef _CELL_CLIENT_HPP
#define _CELL_CLIENT_HPP
#include "Cell.hpp"
//MS
#define  CELLENT_HEART_DEAD_TIME 60000
//指定时间内清空发送缓冲区
#define  CELLENT_SEND_BUFF_TIME 200
//客户端数据类型
class CellClient {
private:
    SOCKET _sock;
    char _szMsgBuf[REVC_BUFF_SIZE]; //第二缓冲区，消息缓冲区
    int _lastPos;//消息缓冲区结尾
    sockaddr_in _addr;
    int _lastSendPos;//消息缓冲区结尾
    char _szSendBuf[SEND_BUFF_SIZE];
    //心跳死亡计时
    time_t _dtHeart;
    //上次发送时间
    time_t _dtSend;
public:
    CellClient(SOCKET sock, sockaddr_in addr) {
        _sock = sock;
        _lastPos = 0;
        _lastSendPos = 0;
        memset(_szMsgBuf, 0, sizeof(_szMsgBuf));
        memset(_szSendBuf, 0, sizeof(_szSendBuf));
        _addr = addr;
        ResetDTHeart();
        ResetDTSend();
    }

    ~CellClient() {
#ifdef _WIN32
        closesocket(_sock);
#else
        close(_sock);
#endif
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

    void SendDataReal(DataHeader* pHd) {
        SendData(pHd);
        SendDataReal();
    }

    //立即发送数据
    int SendDataReal() {
        int ret = SOCKET_ERROR;
        if (_lastSendPos > 0 && SOCKET_ERROR != _sock) {
            ret = send(_sock, _szSendBuf, _lastSendPos, 0);
            _lastSendPos = 0;
            ResetDTSend();
            if (SOCKET_ERROR == ret) {
                return ret;
            }
        }
        return ret;
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
                //重置时间
                ResetDTSend();
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

    void ResetDTHeart() {
        _dtHeart = 0;
    }

    bool CheckHeart(time_t dt) {
        _dtHeart += dt;
        if (_dtHeart >= CELLENT_HEART_DEAD_TIME) {
            //printf("CheckHeart sock : %d , time = %d \n", _sock, _dtHeart);
            return true;
        }
        return false;
    }

    void ResetDTSend() {
        _dtSend = 0;
    }
    //定时发送消息
    void CheckSend(time_t dt) {
        _dtSend += dt;
        if (_dtSend >= CELLENT_SEND_BUFF_TIME) {
            //printf("CheckSend sock : %d , time = %d \n", _sock, _dtSend);
            //立即发送数据
            SendDataReal();
        }
    }
};
#endif
