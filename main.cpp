#include<iostream>
#include "memory_pool.h"

using namespace std;

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

private:
    char s[64];
    static MemoryPool* pool;
};

MemoryPool* MyClass::pool = MemoryPool::GetMemoryPool(sizeof(MyClass));

int main()
{
    for(int i = 0; i < 1000000; ++i)
    {
        MyClass* a = new MyClass;
        MyClass* b = new MyClass;
        MyClass* c = new MyClass;
        delete a; 
        delete b; 
        delete c; 
    }
    return true;
}
