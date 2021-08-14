/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "app_config.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <spdlog/spdlog.h>

bool app_config::deserialize(const boost::property_tree::ptree& tree)
{
	video_path = tree.get<std::string>("config.video_path");
	image_path = tree.get<std::string>("config.image_path");
	log_path   = tree.get<std::string>("config.log_path");
	h264_mode  = tree.get<std::string>("config.h264_mode");

	return true;
}
bool app_config::serialize(boost::property_tree::ptree* const tree)
{
	return false;
}
bool app_config::make_default()
{
	video_path = "/opt/suburbanmarine/opaleye/record/video";
	image_path = "/opt/suburbanmarine/opaleye/record/image";
	log_path   = "/opt/suburbanmarine/opaleye/log";
	h264_mode  = "cpu";
	return true;
}

bool app_config_mgr::deserialize(const boost::filesystem::path& p)
{
	try
	{
		boost::property_tree::read_xml(p.string(), m_tree);
	}
	catch(const std::exception& e)
	{
	    SPDLOG_ERROR("app_config_mgr::deserialize Config parse failed", e.what());
	    return false;
	}

	if(!m_config)
	{
		m_config = std::make_shared<app_config>();
	}
	
	if(!m_config->deserialize(m_tree))
	{
		return false;
	}

	return true;
}
bool app_config_mgr::serialize(const boost::filesystem::path& p)
{
	if(!m_config)
	{
		return false;
	}

	if(!m_config->serialize(&m_tree))
	{
		return false;
	}

	boost::property_tree::write_xml(p.string(), m_tree);

	return true;
}
