# memory_pool

C++11内存池

内存池的作用：
1. 预分配和批量分配内存，减少内存碎片
2. 提升内存分配效率

核心思路：
1. thread_local实现分配器，每个线程有自己的分配器，避免竞争；
2. 每次分配空间时，检查空闲内存链表是否为空，如果为空，则新分配(n * 对象size)的空间，减少内存碎片；
3. 释放空间时，不是调用free，而是将空间作为节点返回到空闲内存链表中。从而可循环使用分配的空间，提升内存分配效率；
4. 可以通过宏CACHE_LINE_ALIGNED配置内存节点是否按照cache_line对齐（使用thread_local实现，实际并不需要这一宏。如果使用memory_barrier实现，可能会用于减少false_sharing）。

# To do:
1. 批量分配待实现。

# 性能测试
目前使用的是多线程加锁，单线程不加锁实现，实测同样的分配逻辑下：

64字节对象，分配和释放3百万次：


内存池不加锁(单线程)：

real    0m0.025s

user    0m0.023s

sys     0m0.001s


内存池加锁：

real    0m0.128s

user    0m0.126s

sys     0m0.000s


普通分配方式(malloc-free)：

real    0m0.060s

user    0m0.057s

sys     0m0.001s


可以看到该场景下，无锁的内存池分配，相比malloc-free时间减少了58.3%，提升效果明显。

加锁实现的内存池反而效率低下。所以，不能使用加锁实现内存池！！！



# 2021.04.24 更新

使用线程私有变量解决加锁性能问题，已解决

# 2021.05.15 更新
1. vector预分配空间，提升效率；
2. 添加内存对齐的选择，增加可配置宏定义；
3. 删去链表的指针ptr，节约内存。

