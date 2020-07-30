#ifndef _CELL_STREAM_HPP_
#define _CELL_STREAM_HPP_
#include<cstdint>

//字节流 byte
class CellStream {
private:
    char* _pBuff; //数据缓冲区
    size_t _nSize; //大小
    int _nWritePos = 0; //已写结尾
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

    template<typename T>
    bool Read(T& n, bool bOffset = true) {
        auto nLen = sizeof(T);
        if (_nReadPos + nLen <= _nSize) {
            memcpy(&n, _pBuff + _nReadPos, nLen);
            if (bOffset) {
                _nReadPos += nLen;
            }
            return true;
        }
        return false;
    }

    //数组
    template<typename T>
    uint32_t ReadArray(T* pArray, uint32_t len) {
        uint32_t nArrLen = 0;
        Read(nArrLen, false);
        //判断能不能放的下
        if (nArrLen < len) {
            //计算数组的实际字节长度
            auto nLen = nArrLen * sizeof(T);
            if (_nReadPos + nLen + sizeof(uint32_t) <= _nSize) {
                //计算已读位置+数组长度所占有空间
                _nReadPos += sizeof(uint32_t);
                memcpy(pArray, _pBuff + _nReadPos, nLen);
                _nReadPos += nLen;
                return nArrLen;
            }
        }
        return 0;
    }

    int8_t ReadInt8() {
        int8_t n = 0;
        Read(n);
        return n;
    }

    int16_t ReadInt16() {
        int16_t n = 0;
        Read(n);
        return n;
    }

    int32_t ReadInt32() {
        int32_t n = 0;
        Read(n);
        return n;
    }
    float ReadFloat() {
        float n = 0.f;
        Read(n);
        return n;

    }
    double ReadDouble() {
        double n = 0.;
        Read(n);
        return n;
    }
    ////write
    template<typename T>
    bool Write(T n) {
        size_t nLen = sizeof(T);
        //判断能不能写入
        if (_nWritePos + nLen <= _nSize) {
            memcpy(_pBuff + _nWritePos, &n, nLen);
            _nWritePos += nLen;
            return true;
        }
        return false;
    }

    //数组
    template<typename T>
    bool WriteArray(T* pData, uint32_t len) {
        auto nLen = sizeof(T)*len;
        if (_nWritePos + nLen + sizeof(uint32_t) <= _nSize) {
            //写入数组长度
            WriteInt32(len);
            memcpy(_pBuff + _nWritePos, pData, nLen);
            _nWritePos += nLen;
            return true;
        }
        return false;
    }

    bool WriteInt8(int8_t n) {
        return Write(n);
    }
    bool WriteInt16(int16_t n) {
        return Write(n);
    }
    bool WriteInt32(int32_t n) {
        return Write(n);
    }
    bool WriteFloat(float n) {
        return Write(n);
    }
    bool WriteDouble(double n) {
        return Write(n);
    }

private:

};

#endif
