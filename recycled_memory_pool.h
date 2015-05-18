#ifndef RECYCLED_MEMORYPOOL_H
#define RECYCLED_MEMORYPOOL_H

#include <deque>

namespace hash_index {

template <class T>
class RecycledMemoryPool {
public:
    RecycledMemoryPool() :
        align_(1),
        block_size_(1024 * 1024),
        used_(0),
        free_(0),
        idx_(0) {
    }

    virtual ~RecycledMemoryPool() {
        for (size_t i = 0; i < mem_blocks_.size(); ++i) {
            ::free(mem_blocks_[i]);
        }
        mem_blocks_.clear();
        recycled_.clear();
    }

    int create() {
        if (mem_blocks_.size() != 0) {
            clear();
            return 0;
        }

        void* p = ::malloc(block_size_);
        if (p == NULL) {
            return -1;
        }
        mem_blocks_.push_back(p);
        free_ = block_size_;
        return 0;
    }

    void recycle(T* t) {
        recycled_.push_back(t);
    }

    T* malloc() {
        size_t size = align(sizeof(T));

        if (recycled_.size() > 0) {
            T* ret = recycled_[recycled_.size() - 1];
            recycled_.resize(recycled_.size() - 1);

            return ret;

        } else if (size <= free_) {
            void* x = (unsigned char*)mem_blocks_[idx_] + used_;
            T* ret = new (x) T;
            used_ += size;
            free_ -= size;

            return ret;

        } else if (size <= block_size_) {
            if (idx_ == mem_blocks_.size() - 1) {
                void* p = ::malloc(block_size_);
                if (p == NULL) {
                    return NULL;
                }
                mem_blocks_.push_back(p);
            }
            ++idx_;

            used_ = size;
            free_ = block_size_ - used_;

            void* x = mem_blocks_[idx_];
            T* ret = new (x) T;
            return ret;

        } else {
            // not support > block_size_;
            return NULL;
        }
    }

    void clear() {
        idx_ = 0;
        used_ = 0;
        free_ = block_size_;
    }

    size_t memory_usage() const {
        return block_size_ * mem_blocks_.size();
    }

    void set_align(size_t align) {
        if (mem_blocks_.size() == 0) {
            align_ = align;
        }
    }

    void set_block_size(size_t block_size) {
        if (mem_blocks_.size() == 0) {
            block_size_ = block_size;
        }
    }

private:
    RecycledMemoryPool(const RecycledMemoryPool&);
    void operator=(const RecycledMemoryPool&);

    size_t align(size_t size) {
        return (size + (align_ - 1)) & ~(align_ - 1);
    }

    size_t align_;

    size_t block_size_;
    size_t used_;
    size_t free_;

    uint32_t idx_;
    std::deque< void* > mem_blocks_;

    std::deque< T* > recycled_;
}; // RecycledMemoryPool

} // namespace hash_index

#endif  // RECYCLED_MEMORYPOOL_H

