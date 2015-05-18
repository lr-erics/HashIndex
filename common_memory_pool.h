#ifndef COMMON_MEMORYPOOL_H
#define COMMON_MEMORYPOOL_H

#include <deque>

namespace hash_index {

class CommonMemoryPool {
public:
    CommonMemoryPool() :
        align_(1),
        block_size_(1024 * 1024),
        used_(0),
        free_(0),
        idx_(0) {
    }

    virtual ~CommonMemoryPool() {
        for (size_t i = 0; i < mem_blocks_.size(); ++i) {
            ::free(mem_blocks_[i]);
        }
        mem_blocks_.clear();
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

    void* malloc(size_t alloc_size) {
        size_t size = align(alloc_size);

        if (size <= free_) {
            void* x = (unsigned char*)mem_blocks_[idx_] + used_;
            used_ += size;
            free_ -= size;

            return x;

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
            return x;

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
    CommonMemoryPool(const CommonMemoryPool&);
    void operator=(const CommonMemoryPool&);

    size_t align(size_t size) {
        return (size + (align_ - 1)) & ~(align_ - 1);
    }

    size_t align_;

    size_t block_size_;
    size_t used_;
    size_t free_;

    uint32_t idx_;
    std::deque< void* > mem_blocks_;

}; // CommonMemoryPool

} // namespace hash_index

#endif  // COMMON_MEMORYPOOL_H

