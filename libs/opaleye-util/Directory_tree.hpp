#pragma once

#include <boost/filesystem/path.hpp>

#include <string>
#include <memory>
#include <iosfwd>

class Directory_tree;
class Directory_tree_node;

class Path_util
{
public:
	static size_t num_elements(const boost::filesystem::path& path)
	{
		return std::distance(path.begin(), path.end());
	}

	static bool trailing_element_is_dir(const boost::filesystem::path& path)
	{
		return path.rbegin()->string() == ".";
	}

	// returns true if base_path is a parent path or identical to req_path
	static bool is_parent_path(const boost::filesystem::path& base_path, const boost::filesystem::path& req_path)
	{
		if( base_path.empty() )
		{
			throw std::domain_error("path must not be empty");
		}

		if( req_path.empty() )
		{
			throw std::domain_error("path must not be empty");
		}

		if( ! base_path.is_absolute() )
		{
			throw std::domain_error("must be absolute path");
		}

		if( ! req_path.is_absolute() )
		{
			throw std::domain_error("must be absolute path");
		}

		//base path must be equal to or shorter than req_path
		if(base_path.size() > req_path.size())
		{
			return false;
		}

		const bool base_path_ends_in_dir = trailing_element_is_dir(base_path);

		auto b_it = base_path.begin();
		auto r_it = req_path.begin();
		for(b_it = base_path.begin(), r_it = req_path.begin(); b_it != base_path.end(); ++b_it, ++r_it)
		{
			//if we end in a dir, are at end of base_path, and have matched so far, we match
			if(base_path_ends_in_dir && (std::next(b_it) == base_path.end()))
			{
				return true;
			}

			//if this segment does not match, we do not match
			if(*b_it != *r_it)
			{
				return false;
			}
		}

		return true;
	}
};

class Directory_tree_node : public std::enable_shared_from_this<Directory_tree_node>
{
	friend Directory_tree;

public:

	typedef std::shared_ptr<Directory_tree_node> ptr;
	typedef std::shared_ptr<const Directory_tree_node> const_ptr;

	static Directory_tree_node::ptr create(const Directory_tree_node::ptr& parent, const boost::filesystem::path& full_path)
	{
		return Directory_tree_node::ptr(new Directory_tree_node(parent, full_path));
	}

	bool has_children() const
	{
		return ! m_children.empty();
	}

	bool is_node_name(const std::string& name) const
	{
		return m_name == name;
	}

	Directory_tree_node::ptr create_child(const boost::filesystem::path& child_path)
	{
		Directory_tree_node::ptr this_node = shared_from_this();

		Directory_tree_node::ptr child_node = Directory_tree_node::create(this_node, child_path);
		
		m_children.emplace(child_path.filename().string(), child_node);

		return child_node;
	}

	Directory_tree_node::ptr get_child_by_name(const std::string& name)
	{
		auto it = m_children.find(name);
		if(it == m_children.end())
		{
			return Directory_tree_node::ptr();
		}

		return it->second;
	}

	size_t get_child_count() const
	{
		return m_children.size();
	}

	Directory_tree_node::ptr get_child_by_idx(const size_t& idx)
	{
		if(idx >= get_child_count())
		{
			return Directory_tree_node::ptr();
		}
	
		auto it = m_children.begin();
		std::next(it, idx);

		return it->second;
	}

protected:

	Directory_tree_node(const Directory_tree_node::ptr& parent, const boost::filesystem::path& full_path)
	{
		m_parent    = parent;
		m_full_path = full_path;
		m_name      = full_path.filename().string();
	}

	//For debugging, this is the full path the node was registered with
	boost::filesystem::path m_full_path;

	//The name of this node, eg filename / leaf name
	//Will be . if this a dir node
	std::string m_name;

	//parent
	std::weak_ptr<Directory_tree_node> m_parent;

	//sibling - needed for no stack no recusion traversal
	// std::weak_ptr<Directory_tree_node> m_sibling;

	//children, if any
	std::map<std::string, std::shared_ptr<Directory_tree_node>> m_children;

	//the payload
	std::function<void ()> m_cb;
};

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

std::ostream& operator<<(std::ostream& os, const Directory_tree& rhs);