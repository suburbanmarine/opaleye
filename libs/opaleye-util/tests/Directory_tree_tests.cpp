#include "Directory_tree.hpp"

#include "gtest/gtest.h"

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