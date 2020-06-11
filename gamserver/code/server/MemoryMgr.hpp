#ifndef _MEMORY_MGR_HPP_
#define _MEMORY_MGR_HPP_
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <mutex>

#define MAX_MEMORY_SIZE 128
class MemoryAlloc;

class MemoryBlock {
public:
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
protected:
    char* _pBuf;
    MemoryBlock* _pHeader;
    size_t _nBlockNum;
    size_t _nSize;
    std::mutex _mutex;
public:    
    MemoryAlloc() {
        _pBuf = nullptr;
        _pHeader = nullptr;
        _nBlockNum = 0;
        _nSize = 0; //(memoryBlock + 实际使用内存大小)
    }

    ~MemoryAlloc() {
        if (_pBuf) {
            free(_pBuf);
        }
    }

    void InitMemory() {
        assert(nullptr == _pBuf);
        if (_pBuf){
            return;
        }
        //计算内存池的大小
        size_t bufSize = (_nSize + sizeof(MemoryBlock)) * _nBlockNum;
        //向系统申请内存
        _pBuf = (char*)malloc(bufSize);
        //初始化内存池
        _pHeader = (MemoryBlock*)_pBuf;
        _pHeader->_bPool = true;
        _pHeader->_nId = 0;
        _pHeader->_nRef = 0;
        _pHeader->_pAlloc = this;
        _pHeader->_pNext = nullptr;

        MemoryBlock* pTemp2 = _pHeader;
        for (size_t i = 1; i < _nBlockNum; i++) {
            MemoryBlock* pTemp = (MemoryBlock*)(_pBuf + i * (_nSize + sizeof(MemoryBlock)));
            pTemp->_bPool = true;
            pTemp->_nId = i;
            pTemp->_nRef = 0;
            pTemp->_pAlloc = this;
            pTemp->_pNext = nullptr;

            pTemp2->_pNext = pTemp;
            pTemp2 = pTemp;
        }
    }

    void* AllocMemory(size_t nSize) {
        std::lock_guard<std::mutex> lg(_mutex);
        if (!_pBuf) {
            InitMemory();
        }

        MemoryBlock* pReturn = nullptr;
        if (!_pHeader){
            pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
            pReturn->_bPool = false;
            pReturn->_nId = -1;
            pReturn->_nRef = 1;
            pReturn->_pAlloc = nullptr;
            pReturn->_pNext = nullptr;
            printf("malloc = %x, id = %d, size = %d \n", pReturn, pReturn->_nId, nSize);
        } else {
            pReturn = _pHeader;
            _pHeader = _pHeader->_pNext;
            pReturn->_nRef = 1;
        }
        
        //printf("malloc = %x, id = %d, size = %d \n", pReturn, pReturn->_nId, nSize);
        return ((char*)pReturn + sizeof(MemoryBlock));
    }

    void FreeMemory(void* pMem) {
        char* pData = (char*)pMem;
        MemoryBlock* pBlock =(MemoryBlock*)(pData - sizeof(MemoryBlock));
        assert(1 == pBlock->_nRef);

        //printf("free = %x, id = %d\n", pBlock, pBlock->_nId);
        if (pBlock->_bPool) {
            std::lock_guard<std::mutex> lg(_mutex);
            if (--pBlock->_nRef != 0) {
                return;
            }
            pBlock->_pNext = _pHeader;
            _pHeader = pBlock;
        } else {
            if (--pBlock->_nRef != 0) {
                return;
            }
            free(pBlock);
            return;
        }
    }
};

template<size_t nSize, size_t nBlockNum>
class MemoryAlloctor:public MemoryAlloc {
public:
    MemoryAlloctor() {
        const size_t n= sizeof(void *);
        _nSize = nSize/n * n + (nSize % n == 0 ? 0:n);
        _nBlockNum = nBlockNum;
    }
    ~MemoryAlloctor() {

    }
};

class MemoryMgr {
private:
    MemoryAlloctor<64, 1000000>_mem64;
    MemoryAlloctor<128, 1000000>_mem128;
    MemoryAlloc* _szAlloc[MAX_MEMORY_SIZE + 1];//内存池映射数组
   //MemoryAlloctor<128, 10>_mem64;
private:
    MemoryMgr() {
        init_szAlloc(0, 64, &_mem64);
        init_szAlloc(65, 128, &_mem128);
    }

    ~MemoryMgr() {

    }
    //初始化内存池映射数组
    void init_szAlloc(int nBengin, int nEnd, MemoryAlloc* pMemA) {
        for (int i = nBengin; i <= nEnd; i++) {
            _szAlloc[i] = pMemA;
        }
    }
public:
    static MemoryMgr& Instance() {
        static MemoryMgr m;
        return m;
    }

    void* AllocMem(size_t nSize) {
        if (nSize <= MAX_MEMORY_SIZE){
            return (MemoryBlock*)_szAlloc[nSize]->AllocMemory(nSize);
        } else {
            MemoryBlock* pReturn = (MemoryBlock*)malloc(nSize + sizeof(MemoryBlock));
            pReturn->_bPool = false;
            pReturn->_nId = -1;
            pReturn->_nRef = 1;
            pReturn->_pAlloc = nullptr;
            pReturn->_pNext = nullptr;
            //printf("malloc = %x, id = %d, size = %d \n", pReturn, pReturn->_nId, nSize);
            void* a = (char*)pReturn + sizeof(MemoryBlock);
            return ((char*)pReturn + sizeof(MemoryBlock));
        }
    }

    void FreeMem(void* pMem) {
        char* pData = (char*)pMem;
        MemoryBlock* pBlock = (MemoryBlock*)(pData - sizeof(MemoryBlock));
        
        if (pBlock->_bPool) {
            pBlock->_pAlloc->FreeMemory(pMem);
        } else {
            if (--pBlock->_nRef == 0) {
                //printf("free = %x, id = %d \n", pBlock, pBlock->_nId);
                free(pBlock);
            }
        }
    }

    void AddRef(void* pMem) {
        char* pData = (char*)pMem;
        MemoryBlock* pBlock = (MemoryBlock*)(pData - sizeof(MemoryBlock));
        ++pBlock->_nRef;
    }

};


#endif
