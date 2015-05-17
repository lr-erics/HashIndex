#ifndef  HASH_RESERVE_INDEX_H_
#define  HASH_RESERVE_INDEX_H_

#include <stdint.h>
#include <functional>
#include <vector>
#include <iostream>
#include "string.h"

#include <google/sparse_hash_map>

#include "recycled_memory_pool.h"

namespace hash_index {

template <class T>
struct reverse_index_node_t {
    reverse_index_node_t(T v): value(v), next(0) {}
    reverse_index_node_t(): next(0) {}
    T                       value;
    reverse_index_node_t*   next;
};

typedef std::function<bool(const void* a, const void* b)> EqualFunc;

template <class Key, class T>

class HashReverseIndex {
    typedef Key                         key_type;
    typedef T                           data_type;
    typedef reverse_index_node_t<T>     value_type;

public:
    HashReverseIndex(EqualFunc func) :
        current_index_num_(0) {
        equal_ = func;
    }
    HashReverseIndex() :
        current_index_num_(0) {
        equal_ = NULL;
    }

    void set_item_equal_func(EqualFunc func) {
        equal_ = func;
    }

    ~HashReverseIndex() {
        index_buckets_.clear();
        recycled_.clear();
        mempool_.clear();
    }

    int create(uint32_t index_num) {
        index_buckets_.reserve(index_num);
        recycled_.reserve(100 * 1024 * 1024);

        if(mempool_.create() != 0) {
            return -1;
        }
        return 0;
    }

    int clear() {
        index_buckets_.clear();
        current_index_num_ = 0;
        return 0;
    }

    int reset() {
        clear();
        mempool_.clear();
        recycled_.clear();
        header_.clear();
        return 0;
    }

    value_type* get_index(const Key& index) const {
        value_type* cur_node = NULL;
        auto iter = header_.find(index);
        if (iter != header_.end()) {
            cur_node = reinterpret_cast<value_type*>(index_buckets_[iter->second]);
        }
        return cur_node;
    }

    value_type* insert(const Key& index, const T& value, bool replace_existed) {
        size_t index_id = 0;
        value_type* index_head = get_index(index, index_id);
        value_type* cur_node = index_head;
        if (cur_node == NULL) {
            // alloc head node of new index
            value_type* new_node = new_index(index);
            if (new_node == NULL) {
                return NULL;
            }
            new_node->value = value;
            return new_node;
        }

        if (replace_existed) {
            bool found = false;
            while (cur_node != NULL) {
                bool ret = equal_(&(cur_node->value), &value);
                if (ret) {
                    found = true;
                    break;
                }
                cur_node = cur_node->next;
            }

            // replace
            if (found) {
                cur_node->value = value;
                return cur_node;
            }
        }

        // allocate new node
        value_type* new_node = alloc_node();
        if (new_node == NULL) {
            return NULL;
        }
        new_node->value = value;
        new_node->next = index_head;
        index_buckets_[index_id] = reinterpret_cast<uint64_t>(new_node);
        return new_node;
    }

    void recycle() {
        for(auto iter = recycled_.begin(); iter != recycled_.end(); ++iter) {
            mempool_.recycle(*iter);
        }
        recycled_.clear();
    }

    value_type* remove(const Key& index, const T& value) {
        size_t index_id = 0;
        value_type* cur_node = get_index(index, index_id);

        value_type* pre_node = NULL;
        while (cur_node != NULL) {
            bool ret = equal_(&(cur_node->value), &value);
            if (ret) {
                if (pre_node == NULL) {
                    index_buckets_[index_id] = reinterpret_cast<uint64_t>(cur_node->next);
                } else {
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

    uint32_t index_size() {
        return current_index_num_;
    }

    size_t memory_usage() {
        return (sizeof(uint64_t) * current_index_num_) + mempool_.memory_usage();
    }

private:
    HashReverseIndex(const HashReverseIndex&);
    void operator=(const HashReverseIndex&);

    value_type* get_index(const Key& index, size_t& index_id) {
        value_type* cur_node = NULL;
        index_id = -1;
        auto iter = header_.find(index);
        if (iter != header_.end()) {
            cur_node = reinterpret_cast<value_type*>(index_buckets_[iter->second]);
            index_id = iter->second;
        }
        return cur_node;
    }

    value_type* new_index(const Key& index) {
        auto iter = header_.find(index);
        // have existed no-empty link
        if (iter != header_.end() && index_buckets_[iter->second] != 0) {
            return NULL;
        }

        value_type* cur_node = alloc_node();
        if (cur_node == NULL) {
            return NULL;
        }

        // have not existed the link
        if (iter == header_.end()) {
            header_[index] = current_index_num_;
            index_buckets_.push_back(reinterpret_cast<uint64_t>(cur_node));
            ++current_index_num_;
            return cur_node;
        }

        // add new node into the link existed
        index_buckets_[iter->second] = reinterpret_cast<uint64_t>(cur_node);
        return cur_node;
    }

    value_type* alloc_node() {
        return mempool_.malloc();
    }

    std::vector<uint64_t> index_buckets_;
    size_t current_index_num_;

    EqualFunc equal_;
    google::sparse_hash_map<Key, size_t> header_;
    std::vector<value_type*> recycled_;
    RecycledMemoryPool<value_type> mempool_;
};

} // namespace hash_index 

#endif  //HASH_RESERVE_INDEX_H_

