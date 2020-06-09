#ifndef _ALLCOTOR_H_
#define _ALLCOTOR_H_
void* operator new[](size_t size);
void operator delete[](void* p);
void* operator new(size_t size);
void operator delete(void* p);

#endif