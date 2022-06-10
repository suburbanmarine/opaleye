/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "config/Opaleye_config_pipeline.hpp"

#include <boost/property_tree/ptree.hpp>

#include <spdlog/spdlog.h>

bool pipeline_config::is_valid() const
{
	return true;
}

bool pipeline_config::deserialize(const boost::property_tree::ptree& tree)
{
	m_tree = tree;
	
	name = tree.get<std::string>("name");
	type = tree.get<std::string>("type");

	return true;
} 
bool pipeline_config::serialize(boost::property_tree::ptree* const tree) const
{
	tree->put<std::string>("name", name);
	tree->put<std::string>("type", type);

	return true;
}
