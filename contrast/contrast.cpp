//用于测试普通分配方式，做对比
#include<stdlib.h>

int main()
{
    for(int i = 0; i < 1000000; ++i)
    {
        void * a = malloc(64);
        void * b = malloc(64);
        void * c = malloc(64);
        free(a);
        free(b);
        free(c);
    }
}
