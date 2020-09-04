#ifndef _CELL_MSG_STREAM_HPP_
#define _CELL_MSG_STREAM_HPP_

#include "CellStream.hpp"
//字节流 byte
class CellRecvStream: public CellStream {

public:
    CellRecvStream(DataHeader* header)
        :CellStream((char*)header, header->dataLen) {
        //Push(header->dataLen);
    }

    CellRecvStream(char* pData, int nSize, bool bDelete = false)
        :CellStream(pData, nSize, bDelete) {
        //Push(nSize);
    }

    uint16_t GetNetCmd() {
        uint16_t cmd = CMD_ERROR;
        Read<uint16_t>(cmd);
        return cmd;
    }

    uint16_t GetNetLength() {
        uint16_t len = 0;
        Read<uint16_t>(len);
        return len;
    }
};


class CellSendStream : public CellStream {

public:
    CellSendStream(char* pData, int nSize, bool bDelete = false)
        :CellStream(pData, nSize, bDelete) {
    }

    CellSendStream(size_t nSize = 1024)
        :CellStream(nSize) {
    }

    void SetNetCmd(uint16_t cmd) {
        Write<uint16_t>(cmd);
        //预先占领消息长度
        Write<uint16_t>(0);
    }

    void Finsh() {
        int pos = Length();
        //长度写到 cmd 之后
        SetWritePos(sizeof(uint16_t));
        Write<uint16_t>(pos);
        SetWritePos(pos);
    }
};

#endif
