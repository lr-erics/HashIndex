#ifndef HASH_INDEX_H_
#define HASH_INDEX_H_

#include <stdint.h>
#include <string.h>
#include <vector>

#include "recycled_memory_pool.h"

namespace hash_index {

const static uint32_t g_prime_list[] = {
    17,  23,  31,  41,  59,
    79,  103,  137,  179,  233,
    307,  401,  523,  683,  907,
    1181,  1543,  2011,  2617,  3407,
    4441,  5779,  7517,  9781,  12721,
    16547,  21517,  27983,  36383,  47303,
    61507,  79967,  103963,  135173,  175727,
    228451,  296987,  386093,  501931,  652541,
    848321,  1102823,  1433681,  1863787,  2422939,
    3149821,  4094791,  5323229,  6920201,  8996303,
    11695231,  15203803,  19764947,  25694447,  33402793,
    43423631,  56450731,  73385953,  95401759,  124022287,
    161228983,  209597693,  272477017,  354220127,  460486217,
    598632137,  778221781, 1011700007, 1315250011, 1710000011,
    2222500003, 2889250009, 3756100079, 4294804999,
};

template <class T>
struct entry_t {
    entry_t(uint64_t k, T v): key(k), value(v), next(0) {}
    entry_t(): key(0), next(0) {}
    uint64_t   key;
    T          value;
    entry_t*   next;
};

template <class T>
class HashIndex {
    typedef T            data_type;
    typedef entry_t<T>   value_type;

public:
    HashIndex() :
        hash_buckets_(NULL),
        fixed_hash_num_(0),
        hash_num_(0) {
    }

    ~HashIndex() {
        if (hash_buckets_ != NULL) {
            free(hash_buckets_);
            hash_buckets_ = NULL;
        }
        recycled_.clear();
        mempool_.clear();
    }

    int create(uint32_t hash_num) {
        if (hash_num != 0) {
            fixed_hash_num_ = find_hash_num(hash_num);
        } else {
            hash_num = g_prime_list[0];
        }

        if (adjust(hash_num) < 0) {
            return -1;
        }

        if(mempool_.create() != 0) {
            return -1;
        }

        recycled_.reserve(100 * 1024 * 1024);

        return 0;
    }

    int clear() {
        if (hash_buckets_ != NULL) {
            bzero(hash_buckets_, sizeof(uint64_t) * hash_num_);
        }

        return 0;
    }

    int reset() {
        clear();
        mempool_.clear();
        recycled_.clear();
        return 0;
    }

    int insert(uint64_t key, const T& value) {
        const uint64_t index = key % hash_num_;
        value_type* cur_node = reinterpret_cast<value_type*>(hash_buckets_[index]);
        bool found = false;
        while (cur_node != NULL) {
            T* ret = equal_get(cur_node, key);
            if (ret) {
                found = true;
                break;
            }
            cur_node = cur_node->next;
        }
        if (found) {
            cur_node->value = value;
            return 0;
        }

        value_type* node = alloc_node();
        if (node == NULL) {
            //add log
            return -1;
        }

        return set_node(node, key, value, index);
    }

    T* seek(uint64_t key) const {
        const uint64_t index = key % hash_num_;
        value_type* cur_node = reinterpret_cast<value_type*>(hash_buckets_[index]);

        while (cur_node != NULL) {
            T* ret = equal_get(cur_node, key);
            if (ret) {
                return ret;
            }
            cur_node = cur_node->next;
        }

        return NULL;
    }

    int recycle() {
        for(auto iter = recycled_.begin(); iter != recycled_.end(); ++iter) {
            mempool_.recycle(*iter);
        }
        recycled_.clear();
        return 0;
    }

    value_type* remove(uint64_t key) {
        const uint64_t index = key % hash_num_;
        value_type* cur_node = reinterpret_cast<value_type*>(hash_buckets_[index]);

        value_type* pre_node = NULL;
        while (cur_node != NULL) {
            T* ret = equal_get(cur_node, key);
            if (ret) {
                if (pre_node == NULL) {
                    hash_buckets_[index] = reinterpret_cast<uint64_t>(cur_node->next);
                }
                else {
                    pre_node->next = cur_node->next;
                }

                recycled_.push_back(cur_node);
                return cur_node;
            }
            pre_node = cur_node;
            cur_node = cur_node->next;
        }
        return NULL;
    }

    uint32_t hash_size() {
        return hash_num_;
    }
    size_t memory_usage() {
        return (sizeof(uint64_t) * hash_num_) + mempool_.memory_usage();
    }

private:
    HashIndex(const HashIndex&);
    void operator=(const HashIndex&);

    int set_node(value_type* node, uint64_t key, const T& value, uint64_t hash_index) {
        if (node == NULL) {
            return -1;
        }
        node->key = key;
        node->value = value;
        node->next = reinterpret_cast<value_type*>(hash_buckets_[hash_index]);

        hash_buckets_[hash_index] = reinterpret_cast<uint64_t>(node);

        return 0;
    }

    value_type* alloc_node() {
        return mempool_.malloc();
    }

    T* equal_get(value_type* node, uint64_t key) const {
        return node->key == key ? &(node->value) : NULL;
    }

    int adjust(uint32_t hash_num) {
        if (hash_num <= hash_num_) {
            return clear();
        }

        if (fixed_hash_num_ != 0) {
            hash_num = fixed_hash_num_;
        } else {
            hash_num = find_hash_num(hash_num);
        }

        if (hash_num <= hash_num_) {
            return clear();
        }

        uint64_t* hash_buckets = (uint64_t*) malloc(sizeof(uint64_t) * hash_num);
        if (hash_buckets == NULL) {
            clear();
            return -1;
        }

        if (hash_buckets_ != NULL) {
            free(hash_buckets_);
            hash_buckets_ = NULL;
        }

        hash_buckets_ = hash_buckets;
        hash_num_ = hash_num;

        return clear();
    }

    uint32_t find_hash_num(uint32_t hash_num) {
        const uint32_t prime_size =
            sizeof(g_prime_list) / sizeof(g_prime_list[0]);

        for (uint32_t i = 0; i < prime_size; ++i) {
            if (g_prime_list[i] >= hash_num) {
                return g_prime_list[i];
            }
        }

        return g_prime_list[prime_size - 1];
    }

    uint64_t* hash_buckets_;
    uint32_t fixed_hash_num_;
    uint32_t hash_num_;

    std::vector<value_type*> recycled_;
    RecycledMemoryPool<value_type> mempool_;
};

} // namespace hash_index 

#endif  // HASH_INDEX_H_

