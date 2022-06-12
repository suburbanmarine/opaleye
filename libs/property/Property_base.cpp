/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "Property_base.hpp"

#include "opaleye-util/Ptree_util.hpp"

#include <boost/property_tree/ptree.hpp>

std::string Property_base::to_json() const
{
    return Ptree_util::ptree_to_json_str(to_ptree());
}

boost::property_tree::ptree Property_base::to_ptree() const
{
    boost::property_tree::ptree tree;

    tree.put("name",  name_);
    tree.put("value", value_to_string());
    tree.put("type",  type_);

    return tree;
}

std::string Property_base::metadata_to_json() const
{
    return Ptree_util::ptree_to_json_str(metadata_to_ptree());
}
boost::property_tree::ptree Property_base::metadata_to_ptree() const
{
    boost::property_tree::ptree tree;

    tree.put("name",  name_);
    tree.put("type",  type_);
    tree.put("desc",  desc_);

    boost::property_tree::ptree constraints;
    this->put_constraints(&constraints);

    tree.put_child("constraints", constraints);

    return tree;
}