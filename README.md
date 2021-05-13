# memory_pool

C++11内存池

内存池的作用：
1. 预分配和批量分配内存，减少内存碎片
2. 提升内存分配效率

# To do:
1. 链表free_memory中的ptr可以去掉，每分配一个对象可减少一个指针分配(4字节或8字节)；
2. 加锁会严重损失性能，使得内存池的内存分配效率反而降低。考虑使用thread_local或memory_barrier解决，
3. 如果使用方案为多线程加锁或memory_barrier，如果追求极致的性能，还得考虑false_sharing的问题。thread_local由于每个线程各自拥有分配器，所以没有类似问题，但是是以空间换时间；
4. 如果追求极致的性能，代码中的STL严格来说都有优化方案和替代方案。比如使用std::unordered_map或替代std::map，std::vector可以预分配空间，或者用C自己实现；
5. 批量分配待实现。

# 性能测试
目前使用的是多线程加锁，单线程不加锁实现，实测同样的分配逻辑下：

64字节对象，分配和释放3百万次：


内存池不加锁(单线程)：

real    0m0.046s

user    0m0.045s

sys     0m0.000s


内存池加锁：

real    0m0.153s

user    0m0.150s

sys     0m0.000s


普通分配方式(malloc-free)：

real    0m0.060s

user    0m0.057s

sys     0m0.001s


可以看到该场景下，无锁的内存池分配，相比malloc-free时间减少了23%，提升效果明显。

加锁实现的内存池反而效率低下。所以，不能使用加锁实现内存池！！！



# 2021.04.24 更新

使用线程私有变量解决加锁性能问题，已解决
