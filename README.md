# HashIndex
常见的内存索引结构，内建内存池，更好的管理内存。另外支持数据结点的删除更新操作。

##common_memory_pool.h
内存池，Not thread-safe。
支持常见的内存申请操作。支持任意size的内存申请操作。

##recycled_memory_pool.h
支持的内存回收的内存池，Not thread-safe。
面向每次都是Fixed-size的内存申请操作场景设计，可进行有效的内存回收，提高内存使用效率，避免产生内存碎片。
**不是面向任意size的内存申请场景设计**

##Hash_reverse_index.h
正排索引结构。Not thread-safe。
1. open-hash. 默认数据结点的Key为uint64。
2. 使用内存池管理数据结点的删除和新增等操作，避免引入内存碎片。

##Hash_reverse_index.h
倒排索引。Not thread-safe。
1. 内部使用单向链表用于管理实际的索引数据。
2. 使用内存池管理数据结点的删除和新增等操作，避免引入内存碎片。

##Hash_reverse_index_ex.h
倒排索引加强版。Not thread-safe。
相对于hash_reverse_index.h, 特别之处在于：
1. 内部使用双向链表用于管理实际的索引数据，同时引入Map管理结点与结点在链表中的地址的关系,加速了链表结点的删除操作。
2. 使用内存池管理数据结点的删除和新增等操作，避免引入内存碎片。
