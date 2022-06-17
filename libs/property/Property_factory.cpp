/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2022 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property_factory.hpp"

#include "property/Property_text.hpp"
#include "property/Property_numeric.hpp"

#include "opaleye-util/Ptree_util.hpp"

#include <boost/property_tree/ptree.hpp>

std::shared_ptr<Property_base> Property_factory::from_ptree(const boost::property_tree::ptree& tree)
{
	if( ! (tree.count("name") && tree.count("value") && tree.count("type")))
	{
		return std::shared_ptr<Property_base>();
	}

	const std::string& prop_type = tree.get<std::string>("type");

    std::shared_ptr<Property_base> prop;
    if(prop_type == "text")
    {
        prop = std::make_shared<Property_text>();
    }
    else if(prop_type == "i8")
    {
        prop = std::make_shared<Property_numeric_i8>();
    }
    else
    {
    	return std::shared_ptr<Property_base>();
    }

    prop->name_ = tree.get<std::string>("name");
    prop->set_value_from_string(tree.get<std::string>("value"));

	prop->desc_ = tree.get<std::string>("desc", "");

	if(tree.count("constraints"))
	{
		prop->set_constraints_from_tree(tree.get_child("constraints"));
	}

    return prop;
}

std::shared_ptr<Property_base> Property_factory::from_json(const std::string& str)
{
    return from_ptree(Ptree_util::json_str_to_ptree(str));
}
