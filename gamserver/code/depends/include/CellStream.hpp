#ifndef _CELL_STREAM_HPP_
#define _CELL_STREAM_HPP_
#include<cstdint>

//字节流 byte
class CellStream {
private:
    char* _pBuff; //数据缓冲区
    size_t _nSize; //大小
    int _nWritePos = 0; //结尾
    int _nReadPos = 0;//已读区域
    bool _bDelete;//删除
public:
    CellStream(char* pData, int nSize, bool bDelete = false) {
        _nSize = nSize;
        _pBuff = pData;
        _bDelete = bDelete;
    }

    CellStream(size_t size = 1024) {
        _nSize = size;
        _pBuff = new char[size];
        _bDelete = true;
    }

    ~CellStream() {
        if (_bDelete && _pBuff) {
            delete[]_pBuff;
            _pBuff = nullptr;
        }
    }
////read
    //int8_t ReadInt8();
    //int16_t ReadInt16();
    //int32_t ReadInt32();
    //float ReadFloat();
    //double ReadDouble();
////write
    template<typename T>
    bool Write(T n) {
        size_t nLen = sizeof(T);
        if (_nWritePos + nLen <= _nSize) {
            memcpy(_pBuff + _nWritePos, &n, nLen);
            _nWritePos += nLen;
            return true;
        }
        return false;
    }
    //bool WriteInt8(int8_t n);
    //bool WriteInt16(int16_t n);
    bool WriteInt32(int32_t n) {
        return Write(n);
    }
    //   bool WriteFloat(float n);
    //bool WriteDouble(double n);
private:

};

#endif
