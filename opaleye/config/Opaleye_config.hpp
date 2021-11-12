/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "config/Opaleye_config_camera.hpp"

#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree_fwd.hpp>

#include <map>
#include <string>

class app_config
{
public:
	boost::filesystem::path	video_path;
	boost::filesystem::path	image_path;
	boost::filesystem::path	log_path;

	std::string h264_mode;

	std::map<std::string, camera_config> camera_configs;
	
	bool make_default();

	bool deserialize(const boost::property_tree::ptree& tree);
	bool serialize(boost::property_tree::ptree* const tree) const;

	std::string to_xml_string() const;
	std::string to_json_string() const;

protected:

};