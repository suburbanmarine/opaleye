/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "config/Opaleye_config.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <spdlog/spdlog.h>

#include <sstream>
#include <stdexcept>

bool app_config::is_valid() const
{
	bool ret = true;

	//verify top level stuff

	//verify cameras
	for(const auto& it : camera_configs)
	{
		ret = ret && it.second.is_valid();
	}

	return true;
}

bool app_config::deserialize(const boost::property_tree::ptree& tree)
{
	video_path = tree.get<std::string>("config.video_path");
	image_path = tree.get<std::string>("config.image_path");
	log_path   = tree.get<std::string>("config.log_path");
	h264_mode  = tree.get<std::string>("config.h264_mode");

	sensors_launch = tree.get<std::string>("config.sensors.launch");

	const boost::property_tree::ptree& cameras_tree = tree.get_child("config.cameras");
	for( const auto& camera_i : cameras_tree)
	{
		camera_config cfg;
		if( ! cfg.deserialize(camera_i.second) )
		{
			SPDLOG_ERROR("app_config::deserialize failed, camera {:s}", camera_i.first);
			return false;
		}

		camera_configs[cfg.name] = cfg;
	}

	return true;
}
bool app_config::serialize(boost::property_tree::ptree* const tree) const
{
	tree->clear();

	tree->put("config.<xmlattr>.verson", "1.0");

	tree->put("config.video_path", video_path);
	tree->put("config.image_path", image_path);
	tree->put("config.log_path", log_path);
	tree->put("config.h264_mode", h264_mode);

	tree->put("config.sensors.launch", sensors_launch);

	{
		boost::property_tree::ptree cameras_tree;
		for(const auto& it : camera_configs)
		{
			boost::property_tree::ptree cameras_i;
			if( ! it.second.serialize(&cameras_i))
			{
				return false;
			}
			cameras_tree.add_child("camera", cameras_i);
		}
		tree->put_child("config.cameras", cameras_tree);
	}

	return true;
}
bool app_config::make_default()
{
	video_path = "/opt/suburbanmarine/opaleye/record/video";
	image_path = "/opt/suburbanmarine/opaleye/record/image";
	log_path   = "/opt/suburbanmarine/opaleye/log";
	h264_mode  = "cpu";

	sensors_launch  = "true";

	camera_config cfg;
	cfg.name          = "cam0";
	cfg.type          = "brio";
	cfg.pipeline.name = "cam0-pipeline";
	cfg.pipeline.type = "brio";

	camera_configs[cfg.name] = cfg;

	return true;
}

std::string app_config::to_xml_string() const
{
	boost::property_tree::ptree temp;

	if( ! serialize(&temp) )
	{
		throw std::domain_error("Could not serialize configuration");
	}

	std::stringstream ss;
	boost::property_tree::write_xml(ss, temp);

	return ss.str();
}
std::string app_config::to_json_string() const
{
	boost::property_tree::ptree temp;

	if( ! serialize(&temp) )
	{
		throw std::domain_error("Could not serialize configuration");
	}

	std::stringstream ss;
	boost::property_tree::write_json(ss, temp);

	return ss.str();
}
