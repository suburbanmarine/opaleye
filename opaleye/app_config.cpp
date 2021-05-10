#include "app_config.hpp"

#include <boost/property_tree/xml_parser.hpp>

bool app_config::deserialize(const boost::property_tree::ptree& tree)
{
	video_path = tree.get<std::string>("config.video_path");
	image_path = tree.get<std::string>("config.image_path");
	h264_mode  = tree.get<std::string>("config.h264_mode");

	return true;
}
bool app_config::serialize(boost::property_tree::ptree* const tree)
{
	return false;
}
bool app_config::make_default()
{
	video_path = "/mnt/video";
	image_path = "/mnt/video";
	h264_mode  = "nv";
	return true;
}

bool app_config_mgr::deserialize(const boost::filesystem::path& p)
{
	boost::property_tree::read_xml(p.string(), m_tree);

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
