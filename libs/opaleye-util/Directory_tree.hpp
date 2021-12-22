#pragma once

#include <boost/filesystem/path.hpp>

#include <memory>

class Directory_tree;
class Directory_tree_node;

class Directory_tree_node : public std::enable_shared_from_this<Directory_tree_node>
{
	friend Directory_tree;

public:

	typedef std::shared_ptr<Directory_tree_node> ptr;

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

	Directory_tree_node::ptr get_child_by_name(const std::string& name) const
	{
		auto it = m_children.find(name);
		if(it == m_children.end())
		{
			return Directory_tree_node::ptr();
		}

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
	std::string m_name;

	//parent
	std::weak_ptr<Directory_tree_node> m_parent;

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
	void set_node(const boost::filesystem::path& full_path)
	{
		Directory_tree_node::ptr curr_node = m_root;

		boost::filesystem::path curr_path;

		for(auto q_it = full_path.begin(); q_it != full_path.end(); ++q_it)
		{
			if(curr_node->is_node_name(q_it->string()))
			{
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
			}
		}
	}

	Directory_tree_node::ptr find_match(const boost::filesystem::path& query_path)
	{
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

	// returns true if base_path is a parent path or identical to req_path
	static bool is_parent_path(const boost::filesystem::path& base_path, const boost::filesystem::path& req_path)
	{
		//base path must be equal to or shorter than req_path
		if(base_path.size() > req_path.size())
		{
			return false;
		}

		auto b_it = base_path.begin();
		auto r_it = req_path.begin();
		for(b_it = base_path.begin(), r_it = req_path.begin(); b_it != base_path.end(); ++b_it, ++r_it)
		{
			if(*b_it != *r_it)
			{
				return false;
			}
		}

		return true;
	}

protected:

	Directory_tree_node::ptr m_root;
};
