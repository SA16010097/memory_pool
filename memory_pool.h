#pragma once
#include<vector>
#include<map>
#include<mutex>
#include<stdint.h>
#include<iostream>

//一次分配n * block_size内存空间
#define N_BLOCK 100

#define ADD_LOCK

using namespace std;

class MemoryPool
{
public:
    explicit MemoryPool(const size_t block_size):free_mem_head(NULL), m_block_size(block_size){}
    ~MemoryPool(){
        #ifdef ADD_LOCK
        std::unique_lock<std::mutex> tmp_lock(m_mutex);
        #endif 
        for(auto& item:mem_allocated) 
            free(item);
    }

    static MemoryPool* GetMemoryPool(const size_t s)
    {
        if(pools.find(s) != pools.end()) 
            return pools[s];
        auto ptr = new MemoryPool(s);
        pools.emplace(s, ptr);
        return ptr; 
    }

    void* AllocMemory()
    {
        #ifdef ADD_LOCK
        std::unique_lock<std::mutex> tmp_lock(m_mutex);
        #endif 
        if(free_mem_head == NULL)
        {
            void* mem = malloc((sizeof(void*) * 2 * m_block_size) * N_BLOCK); 
            if(mem == NULL)
                return NULL;

            mem_allocated.push_back(mem); 

            for(size_t i = 0; i < N_BLOCK; ++i)
            {
                auto free_node = new (reinterpret_cast<void*>(reinterpret_cast<uint64_t>(mem) + (sizeof(void*) * 2 + m_block_size) * i)) node;
                free_node->ptr = reinterpret_cast<void*>(reinterpret_cast<uint64_t>(mem) + (sizeof(void*) * 2 + m_block_size) * i + sizeof(void*) * 2);
                if(i == 0) 
                    free_mem_head = free_node;

                if(i < N_BLOCK - 1)
                {
                    auto next_free_node = new (reinterpret_cast<void*>(reinterpret_cast<uint64_t>(mem) + (sizeof(void*) * 2 + m_block_size) * (i + 1))) node;
                    next_free_node->ptr =  new (reinterpret_cast<void*>(reinterpret_cast<uint64_t>(mem) + (sizeof(void*) * 2 + m_block_size) * (i + 1))) node;
                    free_node->next = next_free_node; 
                } 
            }
        
            auto free_ptr = free_mem_head->ptr;
            free_mem_head = free_mem_head->next;
            return free_ptr; 

        }
        else
        {
            auto free_ptr = free_mem_head->ptr;
            free_mem_head = free_mem_head->next;
            return free_ptr; 
        }
    }

    void FreeMemory(void* mem)
    {
        #ifdef ADD_LOCK
        std::unique_lock<std::mutex> tmp_lock;
        #endif 
        auto tmp = free_mem_head;
        
        auto free_node = (node*)(reinterpret_cast<void*>(reinterpret_cast<uint64_t>(mem) - sizeof(void*) * 2));
        free_mem_head = free_node;
        free_mem_head->next = tmp;
    }

private:

    struct node
    {
        void* ptr;
        struct node* next;

        node():ptr(NULL), next(NULL){};
    };

private:
    node* free_mem_head;
    std::vector<void*> mem_allocated; //用于析构释放内存
    size_t m_block_size;

    #ifdef ADD_LOCK
    std::mutex m_mutex;
    #endif

private:
    static std::map<size_t, MemoryPool*> pools;
};

std::map<size_t, MemoryPool*> MemoryPool::pools;
