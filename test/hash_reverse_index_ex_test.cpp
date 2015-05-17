#include <gtest/gtest.h>
#include <iostream>
#include <stdio.h>

#include "hash_reverse_index_ex.h"

class HashReverseIndexEXTest : public ::testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

struct custom_node_t {
    uint32_t weight;
};

typedef hash_index::reverse_index_node_ex_t<uint64_t, custom_node_t> node_type;

TEST_F(HashReverseIndexEXTest, test_create)
{
    hash_index::HashReverseIndexEX<uint64_t, uint64_t, custom_node_t> index;
    EXPECT_EQ(0, index.create(5000));
}

TEST_F(HashReverseIndexEXTest, test_get_index)
{
    hash_index::HashReverseIndexEX<uint64_t, uint64_t, custom_node_t> index;
    EXPECT_EQ(0, index.create(5000));

    uint64_t city_id = 20;
    size_t index_id = 0;
    node_type* node1 = index.get_index(city_id, index_id);
    EXPECT_EQ(-1, index_id);
    EXPECT_TRUE(node1 == NULL);
    node1 = index.get_index(city_id);
    EXPECT_TRUE(node1 == NULL);

    custom_node_t cnode1 = {30};
    EXPECT_TRUE(NULL != index.insert(city_id, 20, cnode1, false));
    node1 = index.get_index(city_id, index_id);
    EXPECT_EQ(0, index_id);
    EXPECT_TRUE(node1 != NULL);
    EXPECT_TRUE(node1->next == NULL);
    EXPECT_TRUE(node1->pre == NULL);
    EXPECT_TRUE(node1->key == 20);
    EXPECT_TRUE(node1->value.weight == 30);
    EXPECT_EQ(1, index.current_index_num_);
    EXPECT_EQ(reinterpret_cast<uint64_t>(node1), index.index_buckets_[index_id]);

    custom_node_t cnode2 = {40};
    EXPECT_TRUE(NULL != index.insert(city_id, 30, cnode2, false));
    node_type* node2 = index.get_index(city_id, index_id);
    EXPECT_TRUE(node2->key == 30);
    EXPECT_TRUE(node2->value.weight == 40);
    EXPECT_TRUE(node2->next == node1);
    EXPECT_TRUE(node2->pre == NULL);

    EXPECT_EQ(2, index.node_map_.size());
}

TEST_F(HashReverseIndexEXTest, test_new_index)
{
    /*
    hash_index::HashReverseIndexEX<uint64_t, uint64_t, custom_node_t> index;
    EXPECT_EQ(0, index.create(5000));
    EXPECT_EQ(0, index.current_index_num_);
    EXPECT_EQ(0U, index.index_buckets_.size());

    uint64_t index_id1 = 20;
    node_type* node1 = index.new_index(index_id1);
    EXPECT_TRUE(node1 != NULL);
    EXPECT_EQ(1U, index.current_index_num_);
    EXPECT_EQ(1U, index.index_buckets_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node1), index.index_buckets_[0]);

    node_type* node2 = index.new_index(index_id1);
    EXPECT_TRUE(NULL == node2);

    node_type* node3 = index.new_index(index_id1 + 2);
    EXPECT_TRUE(node3 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node1), index.index_buckets_[0]);
    EXPECT_EQ(reinterpret_cast<uint64_t>(node3), index.index_buckets_[1]);
    */
}

TEST_F(HashReverseIndexEXTest, test_insert)
{
    hash_index::HashReverseIndexEX<uint64_t, uint64_t, custom_node_t> index;
    EXPECT_EQ(0, index.create(5000));
    EXPECT_EQ(0, index.current_index_num_);
    EXPECT_EQ(0U, index.index_buckets_.size());

    uint64_t index_id1 = 20;
    custom_node_t cnode1 = {30};
    node_type* node1 = index.insert(index_id1, 20, cnode1, false);
    EXPECT_TRUE(node1 != NULL);
    EXPECT_EQ(1U, index.current_index_num_);
    EXPECT_EQ(1U, index.index_buckets_.size());
    EXPECT_EQ(1U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node1), index.index_buckets_[0]);
    EXPECT_EQ(20, node1->key);
    EXPECT_EQ(30, node1->value.weight);
    EXPECT_TRUE(NULL == node1->next);
    EXPECT_TRUE(NULL == node1->pre);

    uint64_t index_id2 = 30;
    custom_node_t cnode2 = {40};
    node_type* node2 = index.insert(index_id2, 30, cnode2, false);
    EXPECT_TRUE(node2 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(2U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node2), index.index_buckets_[1]);
    EXPECT_EQ(30, node2->key);
    EXPECT_EQ(40, node2->value.weight);
    EXPECT_TRUE(NULL == node2->next);
    EXPECT_TRUE(NULL == node2->next);

    uint64_t index_id3 = 20;
    custom_node_t cnode3 = {90};
    node_type* node3 = index.insert(index_id3, 50, cnode3, false);
    EXPECT_TRUE(node3 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(2U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node3), index.index_buckets_[0]);
    EXPECT_EQ(50, node3->key);
    EXPECT_EQ(90, node3->value.weight);
    EXPECT_TRUE(node1 == node3->next);
    EXPECT_TRUE(NULL == node3->pre);

    uint64_t index_id4 = 30;
    custom_node_t cnode4 = {100};
    node_type* node4 = index.insert(index_id4, 30, cnode4, true);
    EXPECT_TRUE(node4 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(2U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node4), index.index_buckets_[1]);
    EXPECT_EQ(30, node4->key);
    EXPECT_EQ(100, node4->value.weight);
    EXPECT_TRUE(NULL == node4->next);
    EXPECT_TRUE(NULL == node4->pre);
    EXPECT_TRUE(node2 == node4);

    uint64_t index_id6 = 30;
    custom_node_t cnode6 = {100};
    node_type* node6 = index.insert(index_id6, 90, cnode6, true);
    EXPECT_TRUE(node6 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(2U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node6), index.index_buckets_[1]);
    EXPECT_EQ(90, node6->key);
    EXPECT_EQ(100, node6->value.weight);
    EXPECT_TRUE(node4 == node6->next);
    EXPECT_TRUE(NULL == node6->pre);

    uint64_t index_id5 = 30;
    custom_node_t cnode5 = {1001};
    node_type* node5 = index.insert(index_id5, 191, cnode5, false);
    EXPECT_TRUE(node5 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(2U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node5), index.index_buckets_[1]);
    EXPECT_EQ(191, node5->key);
    EXPECT_EQ(1001, node5->value.weight);
    EXPECT_TRUE(node6 == node5->next);
    EXPECT_TRUE(NULL == node5->pre);
    size_t node_cnt = 0;
    while(node5 != NULL) {
        node_cnt++;
        node5 = node5->next;
    }
    EXPECT_EQ(3, node_cnt);
}

TEST_F(HashReverseIndexEXTest, test_remove)
{
    hash_index::HashReverseIndexEX<uint64_t, uint64_t, custom_node_t> index;
    EXPECT_EQ(0, index.create(5000));
    EXPECT_EQ(0, index.current_index_num_);
    EXPECT_EQ(0U, index.index_buckets_.size());

    uint64_t index_id1 = 20;
    custom_node_t cnode1 = {30};
    node_type* node1 = index.insert(index_id1, 20, cnode1, false);
    EXPECT_TRUE(node1 != NULL);
    uint64_t index_id2 = 20;
    custom_node_t cnode2 = {40};
    node_type* node2 = index.insert(index_id2, 50, cnode2, false);
    EXPECT_TRUE(node2 != NULL);
    uint64_t index_id3 = 20;
    custom_node_t cnode3 = {90};
    node_type* node3 = index.insert(index_id3, 50, cnode3, true);
    EXPECT_TRUE(node3 != NULL);
    uint64_t index_id4 = 30;
    custom_node_t cnode4 = {100};
    node_type* node4 = index.insert(index_id4, 300, cnode4, true);
    EXPECT_TRUE(node4 != NULL);
    uint64_t index_id5 = 30;
    custom_node_t cnode5 = {1000};
    node_type* node5 = index.insert(index_id5, 400, cnode5, false);
    EXPECT_TRUE(node5 != NULL);
    uint64_t index_id6 = 30;
    custom_node_t cnode6 = {1001};
    node_type* node6 = index.insert(index_id6, 900, cnode6, true);
    EXPECT_TRUE(node6 != NULL);
    uint64_t index_id7 = 10;
    custom_node_t cnode7 = {1001};
    node_type* node7 = index.insert(index_id7, 11190, cnode7, true);
    EXPECT_TRUE(node7 != NULL);

    node_type* rnode1 = index.remove(index_id1, 20);
    EXPECT_EQ(20, rnode1->key);
    EXPECT_EQ(30, rnode1->value.weight);
    EXPECT_TRUE(NULL == rnode1->next);
    EXPECT_TRUE(node3 == rnode1->pre);
    node_type* v1 = reinterpret_cast<node_type*>(index.index_buckets_[0]);
    EXPECT_TRUE(NULL == v1->next);
    EXPECT_TRUE(NULL == v1->pre);
    EXPECT_EQ(1, index.recycled_.size());
    EXPECT_EQ(rnode1, index.recycled_[0]);
    EXPECT_EQ(50, v1->key);
    EXPECT_EQ(90, v1->value.weight);

    node_type* rnode2 = index.remove(index_id1, 50);
    EXPECT_EQ(50, rnode2->key);
    EXPECT_EQ(90, rnode2->value.weight);
    EXPECT_TRUE(NULL == rnode2->next);
    EXPECT_TRUE(NULL == rnode2->pre);
    EXPECT_EQ(2, index.recycled_.size());
    EXPECT_EQ(rnode1, index.recycled_[0]);
    EXPECT_EQ(rnode2, index.recycled_[1]);
    node_type* v2 = reinterpret_cast<node_type*>(index.index_buckets_[0]);
    EXPECT_TRUE(NULL == v2);

    uint64_t index_id8 = 20;
    custom_node_t cnode8 = {30};
    node_type* node8 = index.insert(index_id8, 40, cnode8, false);
    node_type* v3 = reinterpret_cast<node_type*>(index.index_buckets_[0]);
    EXPECT_TRUE(node8 == v3);
    EXPECT_TRUE(node8->next == NULL);
    EXPECT_TRUE(node8->pre == NULL);
    EXPECT_EQ(40, node8->key);
    EXPECT_EQ(30, node8->value.weight);

    node_type* rnode3 = index.remove(index_id7, 11190);
    EXPECT_EQ(11190, rnode3->key);
    EXPECT_EQ(1001, rnode3->value.weight);
    EXPECT_TRUE(NULL == rnode3->next);
    EXPECT_TRUE(NULL == rnode3->pre);
    EXPECT_EQ(3, index.recycled_.size());
    EXPECT_EQ(rnode1, index.recycled_[0]);
    EXPECT_EQ(rnode2, index.recycled_[1]);
    EXPECT_EQ(rnode3, index.recycled_[2]);
    node_type* v4 = reinterpret_cast<node_type*>(index.index_buckets_[2]);
    EXPECT_TRUE(NULL == v4);
}
