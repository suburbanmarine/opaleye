#pragma once

#include "Directory_tree_fwd.hpp"
#include "Directory_tree_node.hpp"

#include <boost/filesystem/path.hpp>

#include <iosfwd>
#include <iostream>
#include <string>

class Directory_tree
{
public:

	Directory_tree()
	{
		m_root = Directory_tree_node::create(Directory_tree_node::ptr(), "/");
	}

	virtual ~Directory_tree()
	{

	}

	enum MATCH_TYPE
	{
		PARENT_PATH,
		EXACT,
		NONE
	};

	Directory_tree_node::ptr set_node(const boost::filesystem::path& full_path)
	{
		return set_node(full_path, Directory_tree_node::Data::ptr());
	}

	/// if node matches path, set node to this handler
	/// if node does not exist, create a new node
	Directory_tree_node::ptr set_node(const boost::filesystem::path& full_path, const Directory_tree_node::Data::ptr& data)
	{
		if( ! full_path.is_absolute() )
		{
			throw std::domain_error("full_path must be absolute path");
		}

		if( ! m_root->is_node_name(full_path.begin()->string()) )
		{
			return Directory_tree_node::ptr();
		}

		Directory_tree_node::ptr curr_node = m_root;
		boost::filesystem::path curr_path  = m_root->path();

		for(auto q_it = std::next(full_path.begin()); q_it != full_path.end(); ++q_it)
		{
			curr_path = curr_path / *q_it;
				
			Directory_tree_node::ptr child = curr_node->get_child_by_name(q_it->string());
			if( ! child )
			{
				child = curr_node->create_child(curr_path);
			}

			curr_node = child;
		}

		if(curr_node)
		{
			curr_node->set_data(data);
		}

		return curr_node;
	}

	Directory_tree_node::ptr find_match(const boost::filesystem::path& query_path, const MATCH_TYPE mode = MATCH_TYPE::EXACT)
	{
		if( ! query_path.is_absolute() )
		{
			throw std::domain_error("query_path must be absolute path");
		}

		if( ! m_root->is_node_name(query_path.begin()->string()) )
		{
			return Directory_tree_node::ptr();
		}

		Directory_tree_node::ptr curr_node = m_root;
		for(auto q_it = std::next(query_path.begin()); q_it != query_path.end(); ++q_it)
		{
			std::cout << " q_it: " << q_it->string() << "\n";
			Directory_tree_node::ptr next_node = curr_node->get_child_by_name(q_it->string());
			if( ! next_node )
			{
				if(mode == MATCH_TYPE::PARENT_PATH)
				{
					curr_node = curr_node->get_child_by_name(".");
				}
				break;
			}
			curr_node = next_node;
		}

		if(curr_node)
		{
			switch(mode)
			{
				case MATCH_TYPE::PARENT_PATH:
				{
					//exact match ok
					if(curr_node->is_path(query_path))
					{
						return curr_node;
					}
					//check curr_node is dir
					if(Path_util::trailing_element_is_dir(curr_node->m_full_path))
					{
						return curr_node;
					}
					break;
				}
				case MATCH_TYPE::EXACT:
				{
					//check exact match
					if(curr_node->is_path(query_path))
					{
						return curr_node;
					}
					break;
				}
				default:
				{
					throw std::domain_error("mode is oob");
					break;
				}
			}
		}

		//no match
		return Directory_tree_node::ptr();
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

std::ostream& operator<<(std::ostream& os, const Directory_tree& rhs);
