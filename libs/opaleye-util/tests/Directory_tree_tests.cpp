#include "Directory_tree.hpp"

#include "gtest/gtest.h"

TEST(Path_util, is_parent_path)
{
	//no empty path
	EXPECT_THROW(Path_util::is_parent_path("", ""), std::domain_error);
	EXPECT_THROW(Path_util::is_parent_path("/", ""), std::domain_error);
	EXPECT_THROW(Path_util::is_parent_path("", "/"), std::domain_error);

	//no rel path
	EXPECT_THROW(Path_util::is_parent_path("/", "foo"), std::domain_error);
	EXPECT_THROW(Path_util::is_parent_path("foo", "/"), std::domain_error);

	//exact match passes
	EXPECT_TRUE(Path_util::is_parent_path("/", "/"));
	EXPECT_TRUE(Path_util::is_parent_path("/foo", "/foo"));
	EXPECT_TRUE(Path_util::is_parent_path("/foo/bar", "/foo/bar"));
	EXPECT_TRUE(Path_util::is_parent_path("/foo/bar/", "/foo/bar/"));

	//containing folder passes
	EXPECT_TRUE(Path_util::is_parent_path("/", "/foo"));
	EXPECT_TRUE(Path_util::is_parent_path("/", "/foo/bar"));

	//containing folder passes
	EXPECT_TRUE(Path_util::is_parent_path("/a/", "/a/foo/"));
	EXPECT_TRUE(Path_util::is_parent_path("/a/", "/a/foo/bar/"));

	//ending folder does not match
	EXPECT_FALSE(Path_util::is_parent_path("/foo/", "/foo"));


	EXPECT_TRUE(Path_util::is_parent_path("/foo/", "/foo/bar"));


	EXPECT_TRUE(Path_util::is_parent_path("/foo/", "/foo/"));
	EXPECT_TRUE(Path_util::is_parent_path("/foo/", "/foo/bar/"));
}

TEST(Directory_tree, construct)
{
	Directory_tree tree;
}

TEST(Directory_tree, set_node)
{
	Directory_tree tree;

	tree.set_node("/a");

	tree.set_node("/c/d/e");
}
