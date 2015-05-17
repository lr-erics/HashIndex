#include <gtest/gtest.h>
#include <iostream>
#include <stdio.h>

#include "hash_index.h"
#include "recycled_memory_pool.h"

class RecycledMemoryPoolTest : public ::testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

struct custom_node_t {
    uint64_t id;
    uint32_t weight;
};

typedef hash_index::entry_t<custom_node_t> data_type;

TEST_F(RecycledMemoryPoolTest, test_create)
{
    hash_index::RecycledMemoryPool<data_type> pool;

    EXPECT_EQ(1, pool.align_);
    EXPECT_EQ(1024UL * 1024UL, pool.block_size_);
    EXPECT_EQ(0, pool.used_);
    EXPECT_EQ(0, pool.free_);
    EXPECT_EQ(0, pool.idx_);
    EXPECT_EQ(0U, pool.mem_blocks_.size());

    EXPECT_EQ(0, pool.create());
    EXPECT_EQ(1UL, pool.mem_blocks_.size());
    EXPECT_EQ(0, pool.used_);
    EXPECT_EQ(1024 * 1024UL, pool.free_);
    EXPECT_EQ(0, pool.idx_);
}

TEST_F(RecycledMemoryPoolTest, test_align)
{
    hash_index::RecycledMemoryPool<int> pool;

    pool.set_align(1);
    EXPECT_EQ(10, pool.align(10));
    EXPECT_EQ(28, pool.align(28));
    EXPECT_EQ(3, pool.align(3));

    pool.set_align(2);
    EXPECT_EQ(10, pool.align(10));
    EXPECT_EQ(28, pool.align(28));
    EXPECT_EQ(4, pool.align(3));
}

TEST_F(RecycledMemoryPoolTest, test_malloc)
{
    hash_index::RecycledMemoryPool<data_type> pool;

    EXPECT_EQ(0, pool.create());
    data_type* ret = pool.malloc();
    EXPECT_FALSE(ret == NULL);

    EXPECT_EQ(0, pool.idx_);
    EXPECT_EQ(32U, pool.used_);
    EXPECT_EQ(1024 * 1024 - 32, pool.free_);

    uint32_t loop = 1024 * 1024 / 32 - 1;
    for (uint32_t i = 0; i < loop; ++i) {
        data_type* data = pool.malloc();
        EXPECT_FALSE(data == NULL);
    }
    EXPECT_EQ(0, pool.idx_);
    EXPECT_EQ(1024 * 1024U, pool.used_);
    EXPECT_EQ(0, pool.free_);

    ret = pool.malloc();
    EXPECT_FALSE(ret == NULL);

    EXPECT_EQ(1, pool.idx_);
    EXPECT_EQ(32U, pool.used_);
    EXPECT_EQ(1024 * 1024 - 32, pool.free_);

    data_type* d1 = pool.malloc();
    EXPECT_FALSE(ret == NULL);
    EXPECT_EQ(1, pool.idx_);
    EXPECT_EQ(64U, pool.used_);
    EXPECT_EQ(1024 * 1024 - 64, pool.free_);

    pool.recycle(d1);
    EXPECT_EQ(1U, pool.recycled_.size());

    data_type* d2 = pool.malloc();
    EXPECT_EQ(1, pool.idx_);
    EXPECT_EQ(64U, pool.used_);
    EXPECT_EQ(1024 * 1024 - 64, pool.free_);
    EXPECT_EQ(0U, pool.recycled_.size());

    EXPECT_TRUE(d1 == d2);
}

TEST_F(RecycledMemoryPoolTest, test_malloc_empty_data_node)
{
    /*
    struct empty_custom_node_t {
        float x;
        float y;
    }; */
    struct empty_custom_node_t {
    };
    typedef hash_index::entry_t<empty_custom_node_t> empty_data_type;

    hash_index::RecycledMemoryPool<empty_data_type> pool;

    EXPECT_EQ(0, pool.create());
    empty_data_type* ret = pool.malloc();
    EXPECT_FALSE(ret == NULL);

    EXPECT_EQ(0, pool.idx_);
    EXPECT_EQ(24U, pool.used_);
    EXPECT_EQ(1024 * 1024 - 24, pool.free_);
}
