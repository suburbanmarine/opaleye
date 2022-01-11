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

	Directory_tree_node::ptr cd_dir = tree.set_node("/c/d/");
	ASSERT_NE(nullptr, cd_dir);

	Directory_tree_node::ptr cde_dir = tree.set_node("/c/d/e/");
	ASSERT_NE(nullptr, cde_dir);

	EXPECT_THROW(tree.find_match(""), std::domain_error);

	EXPECT_EQ(a, tree.find_match("/a"));
	EXPECT_EQ(cde, tree.find_match("/c/d/e"));

	EXPECT_EQ(cd_dir, tree.find_match("/c/d/"));
	EXPECT_EQ(cde_dir, tree.find_match("/c/d/e/"));

	//parent path gets last parent
	EXPECT_EQ(cd_dir, tree.find_match("/c/d/f", Directory_tree::MATCH_TYPE::PARENT_PATH));
	EXPECT_EQ(cde_dir, tree.find_match("/c/d/e/f", Directory_tree::MATCH_TYPE::PARENT_PATH));

	//parent path prefers exact match
	EXPECT_EQ(cd_dir, tree.find_match("/c/d/", Directory_tree::MATCH_TYPE::PARENT_PATH));
	EXPECT_EQ(cde, tree.find_match("/c/d/e", Directory_tree::MATCH_TYPE::PARENT_PATH));
}