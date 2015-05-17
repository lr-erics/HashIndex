#include <gtest/gtest.h>
#include <iostream>
#include <stdio.h>

#include "hash_reverse_index.h"

class HashReverseIndexTest : public ::testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

struct custom_node_t {
    uint64_t id;
    uint32_t weight;
};

typedef hash_index::reverse_index_node_t<custom_node_t> data_type;

bool my_equal_func(const void* a, const void* b)
{
    const custom_node_t* aa = reinterpret_cast<const custom_node_t*>(a);
    const custom_node_t* bb = reinterpret_cast<const custom_node_t*>(b);
    if (aa->id == bb->id) {
        return true;
    }
    return false;
}

TEST_F(HashReverseIndexTest, test_create)
{
    hash_index::HashReverseIndex<uint64_t, custom_node_t> index(my_equal_func);
    EXPECT_EQ(0, index.create(5000));
}

TEST_F(HashReverseIndexTest, test_get_index)
{
    hash_index::HashReverseIndex<uint64_t, custom_node_t> index(my_equal_func);
    EXPECT_EQ(0, index.create(5000));

    uint64_t city_id = 20;
    size_t index_id = 0;
    data_type* node1 = index.get_index(city_id, index_id);
    EXPECT_EQ(-1, index_id);
    EXPECT_TRUE(node1 == NULL);
    node1 = index.get_index(city_id);
    EXPECT_TRUE(node1 == NULL);

    custom_node_t cnode1 = {20, 30};
    EXPECT_TRUE(NULL != index.insert(city_id, cnode1, false));
    node1 = index.get_index(city_id, index_id);
    EXPECT_EQ(0, index_id);
    EXPECT_TRUE(node1 != NULL);
    EXPECT_TRUE(node1->next == NULL);
    EXPECT_TRUE(node1->value.id == 20);
    EXPECT_TRUE(node1->value.weight == 30);
    EXPECT_EQ(1, index.current_index_num_);
    EXPECT_EQ(reinterpret_cast<uint64_t>(node1), index.index_buckets_[index_id]);

    custom_node_t cnode2 = {30, 40};
    EXPECT_TRUE(NULL != index.insert(city_id, cnode2, false));
    node1 = index.get_index(city_id, index_id);
}

TEST_F(HashReverseIndexTest, test_new_index)
{
    hash_index::HashReverseIndex<uint64_t, custom_node_t> index(my_equal_func);
    EXPECT_EQ(0, index.create(5000));
    EXPECT_EQ(0, index.current_index_num_);
    EXPECT_EQ(0U, index.index_buckets_.size());

    uint64_t index_id1 = 20;
    data_type* node1 = index.new_index(index_id1);
    EXPECT_TRUE(node1 != NULL);
    EXPECT_EQ(1U, index.current_index_num_);
    EXPECT_EQ(1U, index.index_buckets_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node1), index.index_buckets_[0]);

    data_type* node2 = index.new_index(index_id1);
    EXPECT_TRUE(NULL == node2);

    data_type* node3 = index.new_index(index_id1 + 2);
    EXPECT_TRUE(node3 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node1), index.index_buckets_[0]);
    EXPECT_EQ(reinterpret_cast<uint64_t>(node3), index.index_buckets_[1]);
}

TEST_F(HashReverseIndexTest, test_insert)
{
    hash_index::HashReverseIndex<uint64_t, custom_node_t> index(my_equal_func);
    EXPECT_EQ(0, index.create(5000));
    EXPECT_EQ(0, index.current_index_num_);
    EXPECT_EQ(0U, index.index_buckets_.size());

    uint64_t index_id1 = 20;
    custom_node_t cnode1 = {20, 30};
    data_type* node1 = index.insert(index_id1, cnode1, false);
    EXPECT_TRUE(node1 != NULL);
    EXPECT_EQ(1U, index.current_index_num_);
    EXPECT_EQ(1U, index.index_buckets_.size());
    EXPECT_EQ(1U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node1), index.index_buckets_[0]);
    EXPECT_EQ(20, node1->value.id);
    EXPECT_EQ(30, node1->value.weight);
    EXPECT_TRUE(NULL == node1->next);

    uint64_t index_id2 = 30;
    custom_node_t cnode2 = {30, 40};
    data_type* node2 = index.insert(index_id2, cnode2, false);
    EXPECT_TRUE(node2 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(2U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node2), index.index_buckets_[1]);
    EXPECT_EQ(30, node2->value.id);
    EXPECT_EQ(40, node2->value.weight);
    EXPECT_TRUE(NULL == node2->next);

    uint64_t index_id3 = 20;
    custom_node_t cnode3 = {50, 90};
    data_type* node3 = index.insert(index_id3, cnode3, false);
    EXPECT_TRUE(node3 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(2U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node3), index.index_buckets_[0]);
    EXPECT_EQ(50, node3->value.id);
    EXPECT_EQ(90, node3->value.weight);
    EXPECT_TRUE(node1 == node3->next);

    uint64_t index_id4 = 30;
    custom_node_t cnode4 = {30, 100};
    data_type* node4 = index.insert(index_id4, cnode4, true);
    EXPECT_TRUE(node4 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(2U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node4), index.index_buckets_[1]);
    EXPECT_EQ(30, node4->value.id);
    EXPECT_EQ(100, node4->value.weight);
    EXPECT_TRUE(NULL == node4->next);

    uint64_t index_id5 = 30;
    custom_node_t cnode5 = {30, 1000};
    data_type* node5 = index.insert(index_id5, cnode5, false);
    EXPECT_TRUE(node5 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(2U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node5), index.index_buckets_[1]);
    EXPECT_EQ(30, node5->value.id);
    EXPECT_EQ(1000, node5->value.weight);
    EXPECT_TRUE(node4 == node5->next);

    uint64_t index_id6 = 30;
    custom_node_t cnode6 = {90, 1001};
    data_type* node6 = index.insert(index_id6, cnode6, true);
    EXPECT_TRUE(node6 != NULL);
    EXPECT_EQ(2U, index.current_index_num_);
    EXPECT_EQ(2U, index.index_buckets_.size());
    EXPECT_EQ(2U, index.header_.size());
    EXPECT_EQ(reinterpret_cast<uint64_t>(node6), index.index_buckets_[1]);
    EXPECT_EQ(90, node6->value.id);
    EXPECT_EQ(1001, node6->value.weight);
    EXPECT_TRUE(node5 == node6->next);
    size_t node_cnt = 0;
    while(node6 != NULL) {
        node_cnt++;
        node6 = node6->next;
    }
    EXPECT_EQ(3, node_cnt);
}

TEST_F(HashReverseIndexTest, test_remove)
{
    hash_index::HashReverseIndex<uint64_t, custom_node_t> index(my_equal_func);
    EXPECT_EQ(0, index.create(5000));
    EXPECT_EQ(0, index.current_index_num_);
    EXPECT_EQ(0U, index.index_buckets_.size());

    uint64_t index_id1 = 20;
    custom_node_t cnode1 = {20, 30};
    data_type* node1 = index.insert(index_id1, cnode1, false);
    EXPECT_TRUE(node1 != NULL);
    uint64_t index_id2 = 30;
    custom_node_t cnode2 = {30, 40};
    data_type* node2 = index.insert(index_id2, cnode2, false);
    EXPECT_TRUE(node2 != NULL);
    uint64_t index_id3 = 20;
    custom_node_t cnode3 = {50, 90};
    data_type* node3 = index.insert(index_id3, cnode3, false);
    EXPECT_TRUE(node3 != NULL);
    uint64_t index_id4 = 30;
    custom_node_t cnode4 = {30, 100};
    data_type* node4 = index.insert(index_id4, cnode4, true);
    EXPECT_TRUE(node4 != NULL);
    uint64_t index_id5 = 30;
    custom_node_t cnode5 = {30, 1000};
    data_type* node5 = index.insert(index_id5, cnode5, false);
    EXPECT_TRUE(node5 != NULL);
    uint64_t index_id6 = 30;
    custom_node_t cnode6 = {90, 1001};
    data_type* node6 = index.insert(index_id6, cnode6, true);
    EXPECT_TRUE(node6 != NULL);
    uint64_t index_id7 = 10;
    custom_node_t cnode7 = {11190, 1001};
    data_type* node7 = index.insert(index_id7, cnode7, true);
    EXPECT_TRUE(node7 != NULL);

    data_type* rnode1 = index.remove(index_id1, cnode1);
    EXPECT_EQ(20, rnode1->value.id);
    EXPECT_EQ(30, rnode1->value.weight);
    EXPECT_TRUE(NULL == rnode1->next);
    data_type* v1 = reinterpret_cast<data_type*>(index.index_buckets_[0]);
    EXPECT_TRUE(NULL == v1->next);
    EXPECT_EQ(1, index.recycled_.size());
    EXPECT_EQ(rnode1, index.recycled_[0]);
    EXPECT_EQ(50, v1->value.id);
    EXPECT_EQ(90, v1->value.weight);

    data_type* rnode2 = index.remove(index_id1, cnode3);
    EXPECT_EQ(50, rnode2->value.id);
    EXPECT_EQ(90, rnode2->value.weight);
    EXPECT_TRUE(NULL == rnode2->next);
    EXPECT_EQ(2, index.recycled_.size());
    EXPECT_EQ(rnode1, index.recycled_[0]);
    EXPECT_EQ(rnode2, index.recycled_[1]);
    data_type* v2 = reinterpret_cast<data_type*>(index.index_buckets_[0]);
    EXPECT_TRUE(NULL == v2);

    uint64_t index_id8 = 20;
    custom_node_t cnode8 = {900, 30};
    data_type* node8 = index.insert(index_id8, cnode8, false);
    data_type* v3 = reinterpret_cast<data_type*>(index.index_buckets_[0]);
    EXPECT_TRUE(node8 == v3);
    EXPECT_TRUE(node8->next == NULL);
    EXPECT_EQ(900, node8->value.id);
    EXPECT_EQ(30, node8->value.weight);

    data_type* rnode3 = index.remove(index_id7, cnode7);
    EXPECT_EQ(11190, rnode3->value.id);
    EXPECT_EQ(1001, rnode3->value.weight);
    EXPECT_TRUE(NULL == rnode3->next);
    EXPECT_EQ(3, index.recycled_.size());
    EXPECT_EQ(rnode1, index.recycled_[0]);
    EXPECT_EQ(rnode2, index.recycled_[1]);
    EXPECT_EQ(rnode3, index.recycled_[2]);
    data_type* v4 = reinterpret_cast<data_type*>(index.index_buckets_[2]);
    EXPECT_TRUE(NULL == v4);

    data_type* rnode4 = index.remove(index_id2, cnode2);
    EXPECT_EQ(30, rnode4->value.id);
    EXPECT_EQ(1000, rnode4->value.weight);
    EXPECT_TRUE(NULL != rnode4->next);
    EXPECT_EQ(4, index.recycled_.size());
    EXPECT_EQ(rnode1, index.recycled_[0]);
    EXPECT_EQ(rnode2, index.recycled_[1]);
    EXPECT_EQ(rnode3, index.recycled_[2]);
    EXPECT_EQ(rnode4, index.recycled_[3]);

    data_type* rnode5 = index.remove(index_id2, cnode2);
    EXPECT_EQ(30, rnode5->value.id);
    EXPECT_EQ(100, rnode5->value.weight);
    EXPECT_TRUE(NULL == rnode5->next);
    EXPECT_EQ(5, index.recycled_.size());
    EXPECT_EQ(rnode1, index.recycled_[0]);
    EXPECT_EQ(rnode2, index.recycled_[1]);
    EXPECT_EQ(rnode3, index.recycled_[2]);
    EXPECT_EQ(rnode4, index.recycled_[3]);
    EXPECT_EQ(rnode5, index.recycled_[4]);
}
