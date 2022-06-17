/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2022 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property_factory.hpp"

#include "property/Property_text.hpp"

#include "opaleye-util/Ptree_util.hpp"

#include <boost/property_tree/ptree.hpp>

std::shared_ptr<Property_base> Property_factory::from_ptree(const boost::property_tree::ptree& tree)
{
    std::shared_ptr<Property_base> prop;
    if(tree.get<std::string>("type") == "text")
    {
        prop = std::make_shared<Property_text>();
    }

    prop->name_ = tree.get<std::string>("name");
    prop->value_from_string(tree.get<std::string>("value"));

    return prop;
}

std::shared_ptr<Property_base> Property_factory::from_json(const std::string& str)
{
    return from_ptree(Ptree_util::json_str_to_ptree(str));
}
