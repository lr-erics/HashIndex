#ifndef HASH_RESERVE_INDEX_EX_H
#define HASH_RESERVE_INDEX_EX_H

#include <stdint.h>
#include <deque>
#include <vector>
#include <iostream>

#include <google/sparse_hash_map>
#include <unordered_map>

#include "recycled_memory_pool.h"

namespace hash_index {

template <class K, class T>
struct reverse_index_node_ex_t {
    reverse_index_node_ex_t(K k, T v): key(k), value(v), pre(0), next(0) {}
    reverse_index_node_ex_t(): next(0), pre(0) {}
    K                       key;
    T                       value;
    reverse_index_node_ex_t*   next;
    reverse_index_node_ex_t*   pre;
};

template <class Key, class NKT, class NVT>

class HashReverseIndexEX {
    typedef Key                                 key_type;
    typedef NKT                                 node_key_type;
    typedef NVT                                 node_value_type;
    typedef reverse_index_node_ex_t<NKT, NVT>     node_type;

public:
    HashReverseIndexEX() :
        current_index_num_(0) {
    }

    ~HashReverseIndexEX() {
        reset();
    }

    int create(uint32_t index_num) {
        try {
            index_buckets_.resize(index_num);
            index_buckets_.clear();
            recycled_.reserve(100 * 1024 * 1024);

            if(mempool_.create() != 0) {
                return -1;
            }
        } catch(std::bad_alloc& e) {
            return -1;
        } catch(...) {
            return -1;
        }
        return 0;
    }

    int reset() {
        index_buckets_.clear();
        current_index_num_ = 0;
        mempool_.clear();
        recycled_.clear();
        header_.clear();
        node_map_.clear();
        return 0;
    }

    node_type* get_index(const Key& index) const {
        node_type* cur_node = NULL;
        auto iter = header_.find(index);
        if (iter != header_.end()) {
            cur_node = reinterpret_cast<node_type*>(index_buckets_[iter->second]);
        }
        return cur_node;
    }

    node_type* insert(const Key& index, const NKT& key, const NVT& value, bool replace_existed) {
        node_type* node = exist(index, key);
        if (node == NULL) {
            return insert(index, key, value);
        }

        if (node != NULL && replace_existed) {
            node->key = key;
            node->value = value;
            return node;
        }

        return NULL;
    }

    void recycle() {
        for(auto iter = recycled_.begin(); iter != recycled_.end(); ++iter) {
            mempool_.recycle(*iter);
        }
        recycled_.clear();
    }

    node_type* remove(const Key& index, const NKT& key)
    {
        size_t index_id = 0;
        node_type* index_head = get_index(index, index_id);
        if (!index_head) {
            return NULL;
        }

        node_type* target_node = exist(index, key);
        if (!target_node) {
            return NULL;
        }

        std::string xkey = std::to_string(index) + "_" + std::to_string(key);
        node_map_.erase(xkey);
        recycled_.push_back(target_node);
        if (target_node->pre == NULL && target_node->next == NULL) {
            index_buckets_[index_id] = 0;
            return target_node;
        }

        if (target_node->pre == NULL) {
            index_buckets_[index_id] = reinterpret_cast<uint64_t>(target_node->next);
            target_node->next->pre = NULL;
            return target_node;
        }

        if (target_node->next == NULL) {
            target_node->pre->next = NULL;
            return target_node;
        }

        target_node->next->pre = target_node->pre;
        target_node->pre->next = target_node->next;
        return target_node;
    }

    uint32_t index_size() {
        return current_index_num_;
    }

    size_t memory_usage() {
        return (sizeof(uint64_t) * current_index_num_) + mempool_.memory_usage();
    }

private:
    HashReverseIndexEX(const HashReverseIndexEX&);
    void operator=(const HashReverseIndexEX&);

    node_type* get_index(const Key& index, size_t& index_id) {
        node_type* cur_node = NULL;
        index_id = -1;
        auto iter = header_.find(index);
        if (iter != header_.end()) {
            cur_node = reinterpret_cast<node_type*>(index_buckets_[iter->second]);
            index_id = iter->second;
        }
        return cur_node;
    }

    node_type* new_index(const Key& index, const NKT& key, const NVT& value) {
        auto iter = header_.find(index);
        // have existed no-empty link
        if (iter != header_.end() && index_buckets_[iter->second] != 0) {
            return NULL;
        }

        node_type* cur_node = alloc_node();
        if (cur_node == NULL) {
            return NULL;
        }

        cur_node->key = key;
        cur_node->value = value;
        // have not existed the link
        if (iter == header_.end()) {
            header_[index] = current_index_num_;
            index_buckets_.push_back(reinterpret_cast<uint64_t>(cur_node));
            cur_node->pre = NULL;
            cur_node->next = NULL;
            ++current_index_num_;
            return cur_node;
        }

        cur_node->pre = NULL;
        cur_node->next = NULL;
        index_buckets_[iter->second] = reinterpret_cast<uint64_t>(cur_node);
        return cur_node;
    }

    node_type* exist(const Key& index, const NKT& key) const {
        std::string xkey = std::to_string(index) + "_" + std::to_string(key);
        auto iter = node_map_.find(xkey);
        if (iter == node_map_.end()) {
            return NULL;
        }
        return iter->second;
    }

    node_type* alloc_node() {
        return mempool_.malloc();
    }

    node_type* insert(const Key& index, const NKT& key, const NVT& value)
    {
        size_t index_id = 0;
        node_type* index_head = get_index(index, index_id);
        node_type* cur_node = index_head;
        std::string xkey = std::to_string(index) + "_" + std::to_string(key);
        if (cur_node == NULL) {
            // alloc head node of new index
            node_type* new_node = new_index(index, key, value);
            if (new_node == NULL) {
                return NULL;
            }
            node_map_[xkey] = new_node;
            return new_node;
        }

        // allocate new node
        node_type* new_node = alloc_node();
        if (new_node == NULL) {
            return NULL;
        }
        new_node->key = key;
        new_node->value = value;
        index_head->pre = new_node;
        new_node->next = index_head;
        new_node->pre = NULL;
        index_buckets_[index_id] = reinterpret_cast<uint64_t>(new_node);
        node_map_[xkey] = new_node;
        return new_node;
    }

    std::deque<uint64_t> index_buckets_;
    size_t current_index_num_;

    std::unordered_map<std::string, node_type*> node_map_;
    google::sparse_hash_map<Key, size_t> header_;
    std::vector<node_type*> recycled_;
    RecycledMemoryPool<node_type> mempool_;
};

} // namespace hash_index

#endif // HASH_RESERVE_INDEX_EX_H

