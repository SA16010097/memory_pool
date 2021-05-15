#pragma once
#include<vector>
#include<map>
#include<unordered_map>
#include<mutex>
#include<stdint.h>
#include<stdlib.h>
#include<unistd.h>
#include<iostream>
#include<thread>
#include<memory>
#define N_BLOCK 100

#define CACHE_LINE_SIZE (sysconf(_SC_LEVEL1_DCACHE_LINESIZE))

//#define ADD_LOCK

//如果是加锁，或者使用memory barrier实现，则需要考虑false_sharing问题，为此需要考虑按照cache_line对齐
//#define CACHE_LINE_ALIGNED

using namespace std;

class MemoryPool
{
public:
    MemoryPool() = delete;
    
    explicit MemoryPool(const size_t block_size):free_mem_head(NULL)
    {
        //考虑需要按照cache_line对齐，减少false_sharing问题
        #ifdef CACHE_LINE_ALIGNED
        uint32_t cache_line_size = CACHE_LINE_SIZE;
        
        m_block_size = ((block_size + sizeof(void*))/cache_line_size)*cache_line_size;
        if(block_size + sizeof(void*) > m_block_size)
            m_block_size += cache_line_size;
        #else
        m_bolck_size = sizeof(void*) + block_size;
        #endif

        mem_allocated.reserve(10);//预留空间，提升效率 
    }
    
    ~MemoryPool(){
        #ifdef ADD_LOCK
        std::unique_lock<std::mutex> tmp_lock(m_mutex);
        #endif 
        for(auto& item:mem_allocated) 
            free(item);
    }

    void* AllocMemory()
    {
        #ifdef ADD_LOCK
        std::unique_lock<std::mutex> tmp_lock(m_mutex);
        #endif 
        if(free_mem_head == NULL)
        {
            //考虑按cacheline对齐分配
            #ifdef CACHE_LINE_ALIGNED
            void* mem = aligned_alloc(CACHE_LINE_SIZE, m_block_size * N_BLOCK); 
            #else
            void* mem = malloc(m_block_size * N_BLOCK);
            #endif
            
            if(mem == NULL)
                return NULL;

            mem_allocated.push_back(mem); 

            for(size_t i = 0; i < N_BLOCK; ++i)
            {
                auto free_node = new (reinterpret_cast<void*>(reinterpret_cast<uint64_t>(mem) + m_block_size * i)) node;
                if(i == 0) 
                    free_mem_head = free_node;

                if(i < N_BLOCK - 1)
                {
                    auto next_free_node = new (reinterpret_cast<void*>(reinterpret_cast<uint64_t>(mem) + m_block_size * (i + 1))) node;
                    free_node->next = next_free_node; 
                } 
            }
        
            auto free_ptr = reinterpret_cast<void*>(reinterpret_cast<uint64_t>(free_mem_head) + sizeof(void*));
            free_mem_head = free_mem_head->next;
            return free_ptr; 

        }
        else
        {
            auto free_ptr = reinterpret_cast<void*>(reinterpret_cast<uint64_t>(free_mem_head) + sizeof(void*));
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
        
        auto free_node = (node*)(reinterpret_cast<void*>(reinterpret_cast<uint64_t>(mem) - sizeof(void*) ));
        free_mem_head = free_node;
        free_mem_head->next = tmp;
    }

private:

    struct node
    {
        struct node* next;

        node():next(NULL){};
    };

private:
    node* free_mem_head;
    std::vector<void*> mem_allocated; //用于析构释放内存
    size_t m_block_size;

    #ifdef ADD_LOCK
    std::mutex m_mutex;
    #endif

};

thread_local std::unordered_map<size_t, MemoryPool*> pools;
MemoryPool* GetMemoryPool(const size_t s)
{
    if(pools.find(s) != pools.end()) 
        return pools[s];
    auto ptr = new MemoryPool(s);
    pools.emplace(s, ptr);
    return ptr; 
}
