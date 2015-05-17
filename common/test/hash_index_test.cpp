#include <gtest/gtest.h>
#include <iostream>
#include <stdio.h>

#include "hash_index.h"

class HashIndexTest : public ::testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

struct custom_node_t {
    uint64_t id;
    uint32_t weight;
};

typedef hash_index::entry_t<custom_node_t> data_type;
TEST_F(HashIndexTest, test_create)
{
    hash_index::HashIndex<custom_node_t> hash_index;

    EXPECT_TRUE(NULL == hash_index.hash_buckets_);
    EXPECT_EQ(0, hash_index.fixed_hash_num_);
    EXPECT_EQ(0, hash_index.hash_num_);
    EXPECT_EQ(0, hash_index.mempool_.used_);
    EXPECT_EQ(0, hash_index.mempool_.free_);
    EXPECT_EQ(1024 * 1024UL, hash_index.mempool_.block_size_);
    EXPECT_EQ(0, hash_index.mempool_.idx_);
    EXPECT_EQ(0U, hash_index.recycled_.size());

    EXPECT_EQ(0, hash_index.create(50000));
    EXPECT_TRUE(NULL != hash_index.hash_buckets_);
    EXPECT_EQ(61507UL, hash_index.fixed_hash_num_);
    EXPECT_EQ(61507UL, hash_index.hash_num_);
    EXPECT_EQ(0U, hash_index.recycled_.size());
    EXPECT_EQ(0, hash_index.mempool_.used_);
    EXPECT_EQ(1024 * 1024UL, hash_index.mempool_.free_);
    EXPECT_EQ(0, hash_index.mempool_.idx_);
    EXPECT_EQ(1, hash_index.mempool_.mem_blocks_.size());
}

TEST_F(HashIndexTest, test_insert)
{
    hash_index::HashIndex<custom_node_t> hash_index;
    EXPECT_EQ(0, hash_index.create(50000));

    uint64_t test_key1 = 29876;
    custom_node_t test_node1 = {1, 20};
    EXPECT_EQ(0, hash_index.insert(test_key1, test_node1));
    EXPECT_TRUE(0 != hash_index.hash_buckets_[test_key1 % hash_index.hash_num_]);
    data_type* ptr1 = reinterpret_cast<data_type*>(hash_index.hash_buckets_[test_key1 % hash_index.hash_num_]);
    EXPECT_TRUE(NULL != ptr1);
    EXPECT_TRUE(NULL == ptr1->next);
    EXPECT_EQ(29876UL, ptr1->key);
    EXPECT_EQ(1UL, ptr1->value.id);
    EXPECT_EQ(20U, ptr1->value.weight);

    // insert new node
    uint64_t test_key2 = 29876 + 61507;
    custom_node_t test_node2 = {2, 30};
    EXPECT_EQ(0, hash_index.insert(test_key2, test_node2));
    EXPECT_TRUE(0 != hash_index.hash_buckets_[test_key2 % hash_index.hash_num_]);
    data_type* ptr2 = reinterpret_cast<data_type*>(hash_index.hash_buckets_[test_key2 % hash_index.hash_num_]);
    EXPECT_TRUE(NULL != ptr2);
    EXPECT_EQ(29876 + 61507, ptr2->key);
    EXPECT_EQ(2UL, ptr2->value.id);
    EXPECT_EQ(30U, ptr2->value.weight);
    EXPECT_TRUE(ptr1 == ptr2->next);

    // replace
    uint64_t test_key3 = 29876 + 61507;
    custom_node_t test_node3 = {20999, 30};
    EXPECT_EQ(0, hash_index.insert(test_key3, test_node3));
    EXPECT_TRUE(0 != hash_index.hash_buckets_[test_key3 % hash_index.hash_num_]);
    data_type* ptr3 = reinterpret_cast<data_type*>(hash_index.hash_buckets_[test_key3 % hash_index.hash_num_]);
    EXPECT_TRUE(NULL != ptr3);
    EXPECT_EQ(29876 + 61507, ptr3->key);
    EXPECT_EQ(20999UL, ptr3->value.id);
    EXPECT_EQ(30U, ptr3->value.weight);
    EXPECT_TRUE(ptr1 == ptr3->next);

    // add new link
    uint64_t test_key4 = 29879 + 61507;
    custom_node_t test_node4 = {299, 13000};
    EXPECT_EQ(0, hash_index.insert(test_key4, test_node4));
    EXPECT_TRUE(0 != hash_index.hash_buckets_[test_key3 % hash_index.hash_num_]);
    EXPECT_TRUE(0 != hash_index.hash_buckets_[test_key4 % hash_index.hash_num_]);
    data_type* ptr4 = reinterpret_cast<data_type*>(hash_index.hash_buckets_[test_key4 % hash_index.hash_num_]);
    EXPECT_TRUE(NULL != ptr4);
    EXPECT_EQ(29879 + 61507, ptr4->key);
    EXPECT_EQ(299UL, ptr4->value.id);
    EXPECT_EQ(13000U, ptr4->value.weight);
    EXPECT_TRUE(NULL == ptr4->next);
}

TEST_F(HashIndexTest, test_remove)
{
    hash_index::HashIndex<custom_node_t> hash_index;
    EXPECT_EQ(0, hash_index.create(50000));

    uint64_t test_key1 = 29876;
    custom_node_t test_node1 = {1, 128};
    EXPECT_EQ(0, hash_index.insert(test_key1, test_node1));
    uint64_t test_key2 = 29889 + 61507;
    custom_node_t test_node2 = {2, 256};
    EXPECT_EQ(0, hash_index.insert(test_key2, test_node2));
    uint64_t test_key3 = 29887 + 61507;
    custom_node_t test_node3 = {5, 516};
    EXPECT_EQ(0, hash_index.insert(test_key3, test_node3));
    uint64_t test_key4 = 29887 + 61507 * 2;
    custom_node_t test_node4 = {6, 1024};
    EXPECT_EQ(0, hash_index.insert(test_key4, test_node4));
    uint64_t test_key5 = 29887 + 61507 * 5;
    custom_node_t test_node5 = {10, 1098};
    EXPECT_EQ(0, hash_index.insert(test_key5, test_node5));
    uint64_t test_key6 = 29887 + 61507 * 99;
    custom_node_t test_node6 = {11, 2098};
    EXPECT_EQ(0, hash_index.insert(test_key6, test_node6));

    data_type* ret1 = hash_index.remove(test_key3);
    EXPECT_TRUE(NULL != ret1);
    EXPECT_EQ(29887 + 61507UL, ret1->key);
    EXPECT_EQ(5, ret1->value.id);
    EXPECT_EQ(516, ret1->value.weight);
    EXPECT_TRUE(NULL == ret1->next);
    EXPECT_EQ(1U, hash_index.recycled_.size());
    EXPECT_EQ(ret1, hash_index.recycled_[0]);

    data_type* ret2 = hash_index.remove(test_key5);
    EXPECT_TRUE(NULL != ret2);
    EXPECT_EQ(test_key5, ret2->key);
    EXPECT_EQ(test_node5.id, ret2->value.id);
    EXPECT_EQ(test_node5.weight, ret2->value.weight);
    EXPECT_EQ(2U, hash_index.recycled_.size());
    EXPECT_EQ(ret1, hash_index.recycled_[0]);
    EXPECT_EQ(ret2, hash_index.recycled_[1]);
    EXPECT_TRUE(NULL != ret2->next);

    data_type* ret3 = hash_index.remove(test_key1);
    EXPECT_TRUE(NULL != ret3);
    EXPECT_EQ(test_key1, ret3->key);
    EXPECT_EQ(test_node1.id, ret3->value.id);
    EXPECT_EQ(test_node1.weight, ret3->value.weight);
    EXPECT_TRUE(NULL == ret3->next);
    EXPECT_EQ(3U, hash_index.recycled_.size());
    EXPECT_EQ(ret3, hash_index.recycled_[2]);
    data_type* ptr = reinterpret_cast<data_type*>(hash_index.hash_buckets_[test_key1 % hash_index.hash_num_]);
    EXPECT_TRUE(NULL == ptr);
}

TEST_F(HashIndexTest, test_seek)
{
    hash_index::HashIndex<custom_node_t> hash_index;
    EXPECT_EQ(0, hash_index.create(50000));

    uint64_t test_key1 = 29876;
    custom_node_t test_node1 = {1, 128};
    EXPECT_EQ(0, hash_index.insert(test_key1, test_node1));
    uint64_t test_key2 = 29889 + 61507;
    custom_node_t test_node2 = {2, 256};
    EXPECT_EQ(0, hash_index.insert(test_key2, test_node2));
    uint64_t test_key3 = 29887 + 61507;
    custom_node_t test_node3 = {5, 516};
    EXPECT_EQ(0, hash_index.insert(test_key3, test_node3));
    uint64_t test_key4 = 29887 + 61507 * 2;
    custom_node_t test_node4 = {6, 1024};
    EXPECT_EQ(0, hash_index.insert(test_key4, test_node4));
    uint64_t test_key5 = 29887 + 61507 * 5;
    custom_node_t test_node5 = {10, 1098};
    EXPECT_EQ(0, hash_index.insert(test_key5, test_node5));
    uint64_t test_key6 = 29887 + 61507 * 99;
    custom_node_t test_node6 = {11, 2098};
    EXPECT_EQ(0, hash_index.insert(test_key6, test_node6));

    EXPECT_EQ(8 * 61507UL + 1024 * 1024, hash_index.memory_usage());
    EXPECT_EQ(6 * sizeof(data_type), hash_index.mempool_.used_);

    custom_node_t* ret1 = hash_index.seek(test_key3);
    EXPECT_TRUE(NULL != ret1);
    EXPECT_EQ(5, ret1->id);
    EXPECT_EQ(516, ret1->weight);
    custom_node_t* ret2 = hash_index.seek(10);
    EXPECT_TRUE(NULL == ret2);
}

TEST_F(HashIndexTest, test_recycle)
{
    hash_index::HashIndex<custom_node_t> hash_index;
    EXPECT_EQ(0, hash_index.create(50000));

    uint64_t test_key1 = 29876;
    custom_node_t test_node1 = {1, 128};
    EXPECT_EQ(0, hash_index.insert(test_key1, test_node1));
    uint64_t test_key2 = 29889 + 61507;
    custom_node_t test_node2 = {2, 256};
    EXPECT_EQ(0, hash_index.insert(test_key2, test_node2));
    uint64_t test_key3 = 29887 + 61507;
    custom_node_t test_node3 = {5, 516};
    EXPECT_EQ(0, hash_index.insert(test_key3, test_node3));
    uint64_t test_key4 = 29887 + 61507 * 2;
    custom_node_t test_node4 = {6, 1024};
    EXPECT_EQ(0, hash_index.insert(test_key4, test_node4));
    uint64_t test_key5 = 29887 + 61507 * 5;
    custom_node_t test_node5 = {10, 1098};
    EXPECT_EQ(0, hash_index.insert(test_key5, test_node5));
    uint64_t test_key6 = 29887 + 61507 * 99;
    custom_node_t test_node6 = {11, 2098};
    EXPECT_EQ(0, hash_index.insert(test_key6, test_node6));

    data_type* ret1 = hash_index.remove(test_key1);
    data_type* ret2 = hash_index.remove(test_key3);
    EXPECT_EQ(ret1, hash_index.recycled_[0]);
    EXPECT_EQ(ret2, hash_index.recycled_[1]);

    EXPECT_EQ(8 * 61507UL + 1024 * 1024, hash_index.memory_usage());
    EXPECT_EQ(6 * sizeof(data_type), hash_index.mempool_.used_);
    EXPECT_EQ(0, hash_index.mempool_.recycled_.size());

    EXPECT_EQ(0, hash_index.recycle());
    EXPECT_EQ(0, hash_index.recycled_.size());
    EXPECT_EQ(2, hash_index.mempool_.recycled_.size());
    EXPECT_EQ(ret1, hash_index.mempool_.recycled_[0]);
    EXPECT_EQ(ret2, hash_index.mempool_.recycled_[1]);

    // reuse recycled-recently memory blocks
    EXPECT_EQ(0, hash_index.insert(test_key1, test_node1));
    data_type* ret3 = hash_index.remove(test_key1);
    EXPECT_TRUE(ret3 == ret2);
    EXPECT_EQ(1, hash_index.mempool_.recycled_.size());
    EXPECT_EQ(ret1, hash_index.mempool_.recycled_[0]);
}
