#pragma once

#include "Directory_tree_fwd.hpp"
#include "Directory_tree_node.hpp"

#include <boost/filesystem/path.hpp>

#include <string>
#include <iosfwd>

class Directory_tree
{
public:

	Directory_tree()
	{
		m_root = Directory_tree_node::create(Directory_tree_node::ptr(), "/");
	}

	~Directory_tree()
	{

	}

	enum MATCH_TYPE
	{
		PARENT_PATH,
		EXACT,
		NONE
	};

	/// if node matches path, set node to this handler
	/// if node does not exist, create a new node
	Directory_tree_node::ptr set_node(const boost::filesystem::path& full_path)
	{
		if( ! full_path.is_absolute() )
		{
			throw std::domain_error("full_path must be absolute path");
		}

		Directory_tree_node::ptr curr_node = m_root;

		boost::filesystem::path curr_path = "/";

		for(auto q_it = full_path.begin(); q_it != full_path.end(); ++q_it)
		{
			if(curr_node->is_node_name(q_it->string()))
			{
				curr_path = curr_path / *std::next(q_it);
				
				Directory_tree_node::ptr child = curr_node->get_child_by_name(std::next(q_it)->string());
				if(child)
				{
					curr_node = child;
					continue;
				}
				else
				{
					child = curr_node->create_child(curr_path);
				}

				curr_node = child;
			}
			else
			{
				throw std::runtime_error("tree is corrupt");
			}
		}

		return curr_node;
	}

	Directory_tree_node::ptr find_match(const boost::filesystem::path& query_path)
	{
		if( ! query_path.is_absolute() )
		{
			throw std::domain_error("query_path must be absolute path");
		}

		Directory_tree_node::ptr curr_node = m_root;

		for(auto q_it = query_path.begin(); q_it != query_path.end(); ++q_it)
		{
			Directory_tree_node::ptr next_node = curr_node->get_child_by_name(q_it->string());
			if(! next_node )
			{
				break;
			}
			curr_node = next_node;
		}

		return curr_node;
	}

	bool has_children() const
	{
		return m_root->has_children();
	}

	void to_stream(std::ostream& os) const;
	std::string to_string() const;

protected:

	Directory_tree_node::ptr m_root;
};

std::ostream& operator<<(std::ostream& os, const Directory_tree_node& rhs);
std::ostream& operator<<(std::ostream& os, const Directory_tree& rhs);
