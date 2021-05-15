#include<iostream>
#include<stdio.h>
#include "memory_pool.h"

using namespace std;

/*使用举例
class MyClass
{
public:
    MyClass(){}
    ~MyClass(){}

    void* operator new(size_t size)
    {
        return pool->AllocMemory(); 
    }

    void operator delete(void* p)
    {
        pool->FreeMemory(p); 
    }

    int size()
    {
        return sizeof(*this); 
    }
private:
    char s[64];
    static MemoryPool* pool;
};

MemoryPool* MyClass::pool = GetMemoryPool(sizeof(MyClass));
*/

int main()
{
    MemoryPool* pool = GetMemoryPool(64);
    for(int i = 0; i < 1000000; ++i)
    {
        char* a = (char*)pool->AllocMemory(); 
        char* b = (char*)pool->AllocMemory(); 
        char* c = (char*)pool->AllocMemory();
        //printf("%u %u %u\n", a, b, c); 
        pool->FreeMemory((void*)a); 
        pool->FreeMemory((void*)b); 
        pool->FreeMemory((void*)c); 
    }
    return true;
}
