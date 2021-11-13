/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "config/Opaleye_config_pipeline.hpp"

#include <boost/property_tree/ptree_fwd.hpp>

#include <string>

class camera_config
{
public:
	std::string name;
	std::string type;
	pipeline_config pipeline;

	bool is_valid() const;

	bool deserialize(const boost::property_tree::ptree& tree);
	bool serialize(boost::property_tree::ptree* const tree) const;
};
