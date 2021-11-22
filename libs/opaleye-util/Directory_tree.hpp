#pragma once

#include <boost/filesystem/path.hpp>

#include <memory>

class Directory_tree;
class Directory_tree_node;

class Directory_tree_node : public std::enable_shared_from_this<Directory_tree_node>
{
	friend Directory_tree;

public:

	Directory_tree_node(const std::shared_ptr<Directory_tree_node>& parent, const boost::filesystem::path& full_path)
	{
		m_parent    = parent;
		m_full_path = full_path;
		m_name      = full_path.filename().string();
	}

	bool has_children() const
	{
		return ! m_children.empty();
	}

	bool name_match(const std::string& name)
	{
		return m_name == name;
	}

	std::shared_ptr<Directory_tree_node> create_child(const boost::filesystem::path& child_path)
	{
		std::shared_ptr<Directory_tree_node> this_node = shared_from_this();

		std::shared_ptr<Directory_tree_node> child_node = std::make_shared<Directory_tree_node>(this_node, child_path);
		
		m_children.emplace(child_path.filename().string(), child_node);

		return child_node;
	}

	std::shared_ptr<Directory_tree_node> get_child_name_match(const std::string& name)
	{
		auto it = m_children.find(name);
		if(it == m_children.end())
		{
			return std::shared_ptr<Directory_tree_node>();
		}

		return it->second;
	}

protected:

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
		m_root = std::make_shared<Directory_tree_node>(std::shared_ptr<Directory_tree_node>(), "/");
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
		std::shared_ptr<Directory_tree_node> curr_node = m_root;

		boost::filesystem::path curr_path;

		for(auto q_it = full_path.begin(); q_it != full_path.end(); ++q_it)
		{
			if(curr_node->name_match(q_it->string()))
			{
				std::shared_ptr<Directory_tree_node> child = curr_node->get_child_name_match(std::next(q_it)->string());
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

	std::shared_ptr<Directory_tree_node> find_match(const boost::filesystem::path& query_path)
	{
		std::shared_ptr<Directory_tree_node> curr_node = m_root;

		for(auto q_it = query_path.begin(); q_it != query_path.end(); ++q_it)
		{
			std::shared_ptr<Directory_tree_node> next_node = curr_node->get_child_name_match(q_it->string());
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

	std::shared_ptr<Directory_tree_node> m_root;
};
