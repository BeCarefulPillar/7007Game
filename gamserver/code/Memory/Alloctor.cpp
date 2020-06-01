#include "Alloctor.h"
#include <stdlib.h>
#include <stdio.h>
void* operator new[](size_t size) {
    void* pM = malloc(size);
    if (!pM){
        printf("error \n");
    }
    return pM;
}

void operator delete[](void* p) {
    free(p);
}

void* operator new(size_t size) {
    void* pM = malloc(size);
    if (!pM) {
        printf("error \n");
    }
    return pM;
}

void operator delete(void* p) {
    free(p);
}

void* mem_alloc(size_t size) {
    void* pM = malloc(size);
    if (!pM) {
        printf("error \n");
    }
    return pM;
}

void mem_free(void* p) {
    free(p);
}