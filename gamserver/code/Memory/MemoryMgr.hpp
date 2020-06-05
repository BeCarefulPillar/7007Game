#ifndef _MEMORY_MGR_HPP_
#define _MEMORY_MGR_HPP_
#include <stdlib.h>
class MemoryBlock {
private:
    size_t _nId = 0;
    size_t _nRef = 0;
    MemoryAlloc* _pAlloc;
    MemoryBlock* _pNext;
    bool _bPool = false;

public:
    MemoryBlock() {

    }
    ~MemoryBlock() {

    }

};

class MemoryAlloc {
private:
    size_t _nBlockNum;
    size_t _nSize;
    char* _pBuf;
    MemoryBlock* _pHead;
public:    
    MemoryAlloc() {

    }

    ~MemoryAlloc() {

    }

    void InitMemory() {

    }

    void* AllocMem(size_t nSize) {
        return malloc(nSize);
    }

    void FreeMem(void* p) {
        return free(p);
    }
};

class MemoryMgr {
private:
    MemoryMgr() {

    }
    ~MemoryMgr() {

    }
public:
    static MemoryMgr& Instance() {
        static MemoryMgr m;
        return m;
    }

    void* AllocMem(size_t nSize) {
        return malloc(nSize);
    }

    void FreeMem(void* p) {
        return free(p);
    }
};


#endif
