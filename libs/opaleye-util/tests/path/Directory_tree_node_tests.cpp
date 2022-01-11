#include "path/Directory_tree.hpp"

#include "gtest/gtest.h"

TEST(Directory_tree_node, boost_filesystem_is_absolute)
{
	EXPECT_FALSE(boost::filesystem::path("").is_absolute());
	EXPECT_FALSE(boost::filesystem::path("foo").is_absolute());
	EXPECT_FALSE(boost::filesystem::path("foo/bar").is_absolute());

	EXPECT_TRUE(boost::filesystem::path("/").is_absolute());
	EXPECT_TRUE(boost::filesystem::path("/foo").is_absolute());
	EXPECT_TRUE(boost::filesystem::path("/foo/bar").is_absolute());
}

TEST(Directory_tree_node, s)
{

}

TEST(Directory_tree_node, construct_empty)
{
	Directory_tree_node::ptr node_root = Directory_tree_node::create(Directory_tree_node::ptr(), "/");
	EXPECT_NE(nullptr, node_root);

	Directory_tree_node::ptr node2     = Directory_tree_node::create(node_root, "/foo/bar");
	EXPECT_NE(nullptr, node2);
}

TEST(Directory_tree_node, construct_ptr)
{
	Directory_tree_node::ptr node_root = Directory_tree_node::create(Directory_tree_node::ptr(), "/");
	EXPECT_NE(nullptr, node_root);

	Directory_tree_node::ptr node_a    = Directory_tree_node::create(node_root, "/foo");
	EXPECT_NE(nullptr, node_a);

	Directory_tree_node::ptr node_b    = Directory_tree_node::create(node_a, "/foo/bar");
	EXPECT_NE(nullptr, node_b);
}

TEST(Directory_tree_node, has_children)
{
	Directory_tree_node::ptr node_root = Directory_tree_node::create(Directory_tree_node::ptr(), "/");
	ASSERT_NE(nullptr, node_root);

	EXPECT_FALSE(node_root->has_children());

	Directory_tree_node::ptr node_child = node_root->create_child("foo");
	ASSERT_NE(nullptr, node_child);

	EXPECT_TRUE(node_root->has_children());

	EXPECT_FALSE(node_child->has_children());
}

TEST(Directory_tree_node, is_node_name)
{
	Directory_tree_node::ptr node_root = Directory_tree_node::create(Directory_tree_node::ptr(), "/");
	ASSERT_NE(nullptr, node_root);

	EXPECT_TRUE(node_root->is_node_name("/"));
	EXPECT_FALSE(node_root->is_node_name(""));
	EXPECT_FALSE(node_root->is_node_name("foo"));
	EXPECT_FALSE(node_root->is_node_name("/foo"));
}

TEST(Directory_tree_node, create_child)
{
	Directory_tree_node::ptr node_root = Directory_tree_node::create(Directory_tree_node::ptr(), "/");
	ASSERT_NE(nullptr, node_root);

	Directory_tree_node::ptr node_child = node_root->create_child("foo");
	ASSERT_NE(nullptr, node_child);
}
TEST(Directory_tree_node, get_child_by_name)
{
	Directory_tree_node::ptr node_root = Directory_tree_node::create(Directory_tree_node::ptr(), "/");
	ASSERT_NE(nullptr, node_root);

	Directory_tree_node::ptr node_child = node_root->create_child("foo");
	ASSERT_NE(nullptr, node_child);

	Directory_tree_node::ptr node = node_root->get_child_by_name("foo");
	ASSERT_NE(nullptr, node_child);
	EXPECT_EQ(node_child.get(), node.get());
	
	node = node_root->get_child_by_name("");
	ASSERT_EQ(nullptr, node);
	EXPECT_NE(node_child.get(), node.get());

	node = node_root->get_child_by_name("bar");
	ASSERT_EQ(nullptr, node);
	EXPECT_NE(node_child.get(), node.get());
}
