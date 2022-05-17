#include "config/Opaleye_config_mgr.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <spdlog/spdlog.h>

bool app_config_mgr::deserialize(const boost::filesystem::path& p)
{
	try
	{
		boost::property_tree::read_xml(p.string(), m_tree, boost::property_tree::xml_parser::no_comments);
	}
	catch(const std::exception& e)
	{
	    SPDLOG_ERROR("app_config_mgr::deserialize Config XML parse failed: {:s}", e.what());
	    return false;
	}

	if(!m_config)
	{
		m_config = std::make_shared<app_config>();
	}
	
	try
	{
		if(!m_config->deserialize(m_tree))
		{
		    SPDLOG_ERROR("app_config_mgr::deserialize failed");
			return false;
		}
	}
	catch(const std::exception& e)
	{
		SPDLOG_ERROR("app_config_mgr::deserialize failed: {:s}", e.what());
		return false;
	}
	catch(...)
	{
		SPDLOG_ERROR("app_config_mgr::deserialize failed: unknown");
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
