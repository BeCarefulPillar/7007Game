#include "Alloctor.h"
#include "MemoryMgr.hpp"
#include <stdio.h>
void* operator new[](size_t nSize) {
    return MemoryMgr::Instance().AllocMem(nSize);
}

void operator delete[](void* p) {
    return MemoryMgr::Instance().FreeMem(p);
}

void* operator new(size_t nSize) {
    return MemoryMgr::Instance().AllocMem(nSize);
}

void operator delete(void* p) {
    return MemoryMgr::Instance().FreeMem(p);
}
