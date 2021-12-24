#pragma once

#include "Directory_tree_fwd.hpp"
#include "Path_util.hpp"

#include <boost/filesystem/path.hpp>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <iosfwd>

class Directory_tree_node : public std::enable_shared_from_this<Directory_tree_node>
{
	friend Directory_tree;

public:

	class Data
	{
		public:

		typedef std::shared_ptr<Data> ptr;
		typedef std::shared_ptr<const Data> const_ptr;

		Data()
		{

		}

		virtual ~Data()
		{

		}
	};

	typedef std::shared_ptr<Directory_tree_node> ptr;
	typedef std::shared_ptr<const Directory_tree_node> const_ptr;

	virtual ~Directory_tree_node()
	{

	}

	static Directory_tree_node::ptr create(const Directory_tree_node::ptr& parent, const boost::filesystem::path& full_path)
	{
		return Directory_tree_node::ptr(new Directory_tree_node(parent, full_path));
	}

	static Directory_tree_node::ptr create(const Directory_tree_node::ptr& parent, const boost::filesystem::path& full_path, const Data::ptr& data)
	{
		return Directory_tree_node::ptr(new Directory_tree_node(parent, full_path, data));
	}

	void set_data(const Data::ptr& data)
	{
		m_data = data;
	}

	const std::string& node_name() const
	{
		return m_name;
	}

	const boost::filesystem::path& path() const
	{
		return m_full_path;
	}

	bool has_children() const
	{
		return ! m_children.empty();
	}

	bool is_path(const boost::filesystem::path& full_path) const
	{
		return m_full_path == full_path;
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

	void to_stream(std::ostream& os) const;
	std::string to_string() const;
 
	template <typename T> 
	std::shared_ptr<T> get_data()
	{
		return std::dynamic_pointer_cast<T>(m_data);
	}

	template <typename T> 
	std::shared_ptr<const T> get_data() const
	{
		return std::dynamic_pointer_cast<const T>(m_data);
	}

protected:

	Directory_tree_node(const Directory_tree_node::ptr& parent, const boost::filesystem::path& full_path)
	{
		m_parent    = parent;
		m_full_path = full_path;
		m_name      = full_path.filename().string();
	}

	Directory_tree_node(const Directory_tree_node::ptr& parent, const boost::filesystem::path& full_path, const Data::ptr& data)
	{
		m_parent    = parent;
		m_full_path = full_path;
		m_name      = full_path.filename().string();
		m_data      = data;
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
	std::map<std::string, Directory_tree_node::ptr> m_children;

	//the payload
	Data::ptr m_data;
};

std::ostream& operator<<(std::ostream& os, const Directory_tree_node& rhs);
