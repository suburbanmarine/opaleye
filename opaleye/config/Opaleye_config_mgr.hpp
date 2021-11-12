#pragma once

#include "config/Opaleye_config.hpp"

#include <boost/property_tree/ptree.hpp>

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
