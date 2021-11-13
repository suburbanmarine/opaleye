/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "config/Opaleye_config_camera.hpp"

#include <boost/property_tree/ptree.hpp>

#include <spdlog/spdlog.h>

bool camera_config::is_valid() const
{
	return pipeline.is_valid();	
}

bool camera_config::deserialize(const boost::property_tree::ptree& tree)
{
	name = tree.get<std::string>("name");
	type = tree.get<std::string>("type");

	const boost::property_tree::ptree& pipeline_tree = tree.get_child("pipeline");
	if( ! pipeline.deserialize(pipeline_tree) )
	{
		return false;
	}

	return true;
}
bool camera_config::serialize(boost::property_tree::ptree* const tree) const
{
	tree->put<std::string>("name", name);
	tree->put<std::string>("type", type);

	boost::property_tree::ptree temp;
	if( ! pipeline.serialize(&temp))
	{
		return false;
	}
	tree->put_child("pipeline", temp);

	return true;
}
