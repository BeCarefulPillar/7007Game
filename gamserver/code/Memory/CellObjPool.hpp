#ifndef _CELL_OBJ_POOL_HPP_
#define _CELL_OBJ_POOL_HPP_
#include <mutex>
#include <assert.h>
#include <stdio.h>

template<class T, size_t nPoolNum>
class CellObjectPool {
private:
    class NodeHead {
    public:
        size_t _nId = 0;
        size_t _nRef = 0;
        NodeHead* _pNext;
        bool _bPool = false;
    };

    char* _pBuf;
    NodeHead* _pHeader;
    std::mutex _mutex;

private:
    void InitObjPool() {
        assert(nullptr == _pBuf);
        if (_pBuf) {
            return;
        }
        //计算内存池的大小
        size_t realSize = sizeof(T) + sizeof(NodeHead);
        size_t bufSize = realSize* nPoolNum;
        //向系统申请内存
        _pBuf = new char[bufSize];
        //初始化内存池
        _pHeader = (NodeHead*)_pBuf;
        _pHeader->_bPool = true;
        _pHeader->_nId = 0;
        _pHeader->_nRef = 0;
        _pHeader->_pNext = nullptr;

        NodeHead* pTemp2 = _pHeader;
        for (size_t i = 1; i < nPoolNum; i++) {
            NodeHead* pTemp = (NodeHead*)(_pBuf + i * realSize);
            pTemp->_bPool = true;
            pTemp->_nId = i;
            pTemp->_nRef = 0;
            pTemp->_pNext = nullptr;

            pTemp2->_pNext = pTemp;
            pTemp2 = pTemp;
        }
        printf("InitObjPool\n");
    }
public:
    CellObjectPool() {
        _pBuf = nullptr;
        _pHeader = nullptr;
        InitObjPool();
    }

    ~CellObjectPool() {
        if (_pBuf) {
            delete[] _pBuf;
        }
        printf("~CellObjectPool\n");
    }

    void* AllocObjMemory(size_t nSize) {
        std::lock_guard<std::mutex> lg(_mutex);
        if (!_pBuf) {
            InitObjPool();
        }

        NodeHead* pReturn = nullptr;
        if (!_pHeader) {
            pReturn = (NodeHead*)new char[sizeof(T) + sizeof(NodeHead)];
            pReturn->_bPool = false;
            pReturn->_nId = -1;
            pReturn->_nRef = 1;
            pReturn->_pNext = nullptr;
        } else {
            pReturn = _pHeader;
            _pHeader = _pHeader->_pNext;
            pReturn->_nRef = 1;
        }

        printf("AllocObjMemory = %x, id = %d, size = %d \n", pReturn, pReturn->_nId, nSize);
        return ((char*)pReturn + sizeof(NodeHead));
    }

    void FreeObjMemory(void* pMem) {
        char* pData = (char*)pMem;
        NodeHead* pHead = (NodeHead*)(pData - sizeof(NodeHead));
        assert(1 == pHead->_nRef);

        printf("FreeObjMemory = %x, id = %d\n", pHead, pHead->_nId);
        if (pHead->_bPool) {
            std::lock_guard<std::mutex> lg(_mutex);
            if (--pHead->_nRef != 0) {
                return;
            }
            pHead->_pNext = _pHeader;
            _pHeader = pHead;
        } else {
            if (--pHead->_nRef != 0) {
                return;
            }
            delete []pHead;
            return;
        }
    }
};



template<class T, size_t nPoolNum>
class ObjectPoolBase{
public:
    void* operator new(size_t nSize) {
        return ObjectPool().AllocObjMemory(nSize);
    }

    void operator delete(void* p) {
        ObjectPool().FreeObjMemory(p);
    }

    template<typename ...Args> //不定参数
    static T* CreateObj(Args... arg) {
        T* obj = new T(arg...);
        return obj;

    }

    static void DestoryObj(T* obj) {
        delete obj;
    }

private:
    typedef CellObjectPool<T, nPoolNum> ClassTypePool;
    static ClassTypePool& ObjectPool() {
        static ClassTypePool sPool;
        return sPool;
    }

};

#endif
