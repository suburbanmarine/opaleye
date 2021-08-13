/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>

class app_config
{
public:
	boost::filesystem::path	video_path;
	boost::filesystem::path	image_path;
	boost::filesystem::path	log_path;

	std::string h264_mode;
	
	bool make_default();

	bool deserialize(const boost::property_tree::ptree& tree);
	bool serialize(boost::property_tree::ptree* const tree);

protected:

};

class app_config_mgr
{
public:
	bool deserialize(const boost::filesystem::path& p);
	bool serialize(const boost::filesystem::path& p);


	std::shared_ptr<app_config> get_config()
	{
		return m_config;
	}

protected:
	boost::property_tree::ptree m_tree;

	std::shared_ptr<app_config> m_config;
};
