/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <boost/property_tree/ptree.hpp>

#include <string>

class pipeline_config
{
public:
	std::string name;
	std::string type;

	bool is_valid() const;

	bool deserialize(const boost::property_tree::ptree& tree);
	bool serialize(boost::property_tree::ptree* const tree) const;

	template<typename T>
	T get(const boost::property_tree::ptree::path_type& path) const
	{
		return m_tree.get<T>(path);
	}

	template<typename T>
	T get(const boost::property_tree::ptree::path_type& path, const T& default_value) const
	{
		return m_tree.get<T>(path, default_value);
	}

protected:
  // updated by deserialize
  boost::property_tree::ptree m_tree;
};
