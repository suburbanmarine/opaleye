/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "config/Opaleye_config_camera.hpp"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>

#include <map>
#include <string>

class app_config
{
public:
	boost::filesystem::path	video_path;
	boost::filesystem::path	image_path;
	boost::filesystem::path	log_path;

	std::string h264_mode;

	std::string sensors_launch;

	std::string zeromq_launch;
	std::list<std::string> zeromq_ep;

	std::map<std::string, camera_config> camera_configs;
	
	bool is_valid() const;

	bool make_default();

	bool deserialize(const boost::property_tree::ptree& tree);
	bool serialize(boost::property_tree::ptree* const tree) const;

	std::string to_xml_string() const;
	std::string to_json_string() const;

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

	size_t count(const boost::property_tree::ptree::key_type& key) const
	{
		return m_tree.count(key);	
	}

protected:

  // updated by deserialize
  boost::property_tree::ptree m_tree;

};