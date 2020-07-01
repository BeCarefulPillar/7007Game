#ifndef _CELL_CLIENT_HPP
#define _CELL_CLIENT_HPP
#include "Cell.hpp"
#include "CellBuffer.hpp"
//MS
#define  CELLENT_HEART_DEAD_TIME 60000
//指定时间内清空发送缓冲区
#define  CELLENT_SEND_BUFF_TIME 200
//客户端数据类型
class CellClient {
private:
    SOCKET _sock;
    sockaddr_in _addr;
    CellBuffer _sendBuff;
    CellBuffer _recvBuff;
    //心跳死亡计时
    time_t _dtHeart;
    //上次发送时间
    time_t _dtSend;
public:
    CellClient(SOCKET sock, sockaddr_in addr):
        _sendBuff(SEND_BUFF_SIZE), _recvBuff(REVC_BUFF_SIZE){
        _sock = sock;
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

    int RecvData() {
        return _recvBuff.Read2Socket(_sock);
    }

    bool HasMsg() {
        return _recvBuff.HasMsg();
    }

    DataHeader* FrontMsg() {
        return (DataHeader*)_recvBuff.Data();
    }
    
    void PopFrontMsg() {
        if (HasMsg()) {
            _recvBuff.Pop(FrontMsg()->dataLen);
        }
    }

    void SendDataReal(DataHeader* pHd) {
        SendData(pHd);
        SendDataReal();
    }

    //立即发送数据
    int SendDataReal() {
        int ret = _sendBuff.Write2Socket(_sock);
        ResetDTSend();
        return ret;
    }

    int SendData(DataHeader* hd) {
        int ret = SOCKET_ERROR;
        if (!hd) {
            return ret;
        }

        int nSendLen = hd->dataLen;
        const char* pSendData = (const char*)hd;
        if (_sendBuff.Push((const char*)hd, hd->dataLen)) {
            return hd->dataLen;
        }

        return ret;
    }

    void ResetDTHeart() {
        _dtHeart = 0;
    }

    bool CheckHeart(time_t dt) {
        _dtHeart += dt;
        if (_dtHeart >= CELLENT_HEART_DEAD_TIME) {
            //CellLog::Info("CheckHeart sock : %d , time = %d \n", _sock, _dtHeart);
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
            //CellLog::Info("CheckSend sock : %d , time = %d \n", _sock, _dtSend);
            //立即发送数据
            SendDataReal();
        }
    }
};
#endif
