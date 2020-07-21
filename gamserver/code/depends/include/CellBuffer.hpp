#ifndef _CELL_BUFFER_HPP
#define _CELL_BUFFER_HPP
#include "Cell.hpp"

class CellBuffer {
private:
    char* _pBuff;
    int _nLastPos; //结尾
    size_t _nSize; //大小
    //发送缓冲区写满次数
    int _fullCount = 0;
public:
    CellBuffer(size_t size = 8192) {
        _nSize = size;
        _nLastPos = 0;
        _pBuff = new char[size];
    }

    ~CellBuffer() {
        delete[]_pBuff;
        _pBuff = nullptr;
    }

    bool Push(const char* pData, int nLen){ 
        if (!pData) {
            return false;
        }
        //可写数据库、磁盘，超过的数据
        //if (_nLastPos + nLen > _nSize){
        //    int n = _nLastPos + nLen - _nSize;
        //    if (n < 8192) {
        //        n = 8192;
        //    }
        //    char* pData = new char[n + _nSize];
        //    memcpy(pData, _pBuff, _nLastPos);
        //    delete[]_pBuff;
        //    _pBuff = pData;
        //    _nSize += n;
        //}

        if (_nLastPos + nLen <= (int)_nSize) {
            memcpy(_pBuff + _nLastPos, pData, nLen);
            _nLastPos += nLen;
            if (_nLastPos + nLen == (int)_nSize) {
                _fullCount++;
            }
            return true;
        } else {
            _fullCount++;
            CellLog::Info("send full count\n");
        }
        return false;
    }

    int Write2Socket(SOCKET sock) {
        int ret = 0;
        if (_nLastPos > 0 && INVALID_SOCKET != sock) {
            ret = send(sock, _pBuff, _nLastPos, 0);
            _fullCount = 0;
            _nLastPos = 0;
        }
        return ret;
    }

    int Read2Socket(SOCKET sock) {
        if (_nSize - _nLastPos > 0) {
            char* szRevc = _pBuff + _nLastPos;
            int nLen = recv(sock, szRevc, _nSize - _nLastPos, 0); 
            if (nLen <= 0) {
                return nLen;
            }
            //消息缓冲区数据尾部向后
            _nLastPos += nLen;
            return nLen;
        }
        return 0;
    }

    bool HasMsg() {
        if (_nLastPos >= sizeof(DataHeader)) {
            //当前消息
            DataHeader *hd = (DataHeader*)_pBuff;
            if (_nLastPos >= hd->dataLen) {
                return true;
            }
        }
        return false;
    }

    bool NeedWrite() {
        return _nLastPos > 0;
    }

    char* Data() {
        if (_nLastPos > 0) {
            return _pBuff;
        }
        return nullptr;
    }

    void Pop(int nLen) {
        int n = _nLastPos - nLen;
        if (n > 0) {
            memcpy(_pBuff, _pBuff + nLen, n);
        }
        _nLastPos = n;
        if (_fullCount > 0) {
            _fullCount--;
        }
    }

};


#endif
