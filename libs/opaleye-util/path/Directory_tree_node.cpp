#include "Directory_tree_node.hpp"

#include <sstream>
#include <ostream>

void Directory_tree_node::to_stream(std::ostream& os) const
{
	// size_t child_count = m_root->get_child_count();
	// for(size_t i = 0; i < child_count; i++)
	// {
	// 	Directory_tree_node::ptr curr_node = get_child_by_idx
	// }
}

std::string Directory_tree_node::to_string() const
{
	std::stringstream ss;
	to_stream(ss);
	return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Directory_tree_node& rhs)
{
	rhs.to_stream(os);
	return os;
}
