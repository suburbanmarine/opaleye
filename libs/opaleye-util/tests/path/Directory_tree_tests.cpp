#include "path/Directory_tree.hpp"

#include "gtest/gtest.h"

TEST(Directory_tree, construct)
{
	Directory_tree tree;
}

TEST(Directory_tree, set_node)
{
	Directory_tree tree;

	EXPECT_NE(nullptr, tree.set_node("/a"));
	EXPECT_NE(nullptr, tree.set_node("/c/d/e"));
}

TEST(Directory_tree, get_node)
{
	Directory_tree tree;

	Directory_tree_node::ptr a   = tree.set_node("/a");
	ASSERT_NE(nullptr, a);

	Directory_tree_node::ptr cde = tree.set_node("/c/d/e");
	ASSERT_NE(nullptr, cde);

	EXPECT_THROW(tree.find_match(""), std::domain_error);

	EXPECT_EQ(a, tree.find_match("/a"));
	EXPECT_EQ(cde, tree.find_match("/c/d/e"));
}