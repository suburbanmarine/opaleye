/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property_base.hpp"

#include "opaleye-util/Ptree_util.hpp"

#include <boost/property_tree/ptree.hpp>

std::string Property_base::to_json(const bool include_metadata) const
{
    return Ptree_util::ptree_to_json_str(to_ptree(include_metadata));
}

boost::property_tree::ptree Property_base::to_ptree(const bool include_metadata) const
{
    boost::property_tree::ptree tree;

    tree.put("name",  name_);
    tree.put("value", value_to_string());
    tree.put("type",  type_);

    if(include_metadata)
    {
        //metadata
        tree.put("desc",  desc_);
        
        boost::property_tree::ptree constraints;
        this->put_constraints(&constraints);
        tree.put_child("constraints", constraints);
    }

    return tree;
}

std::string Property_base::metadata_to_json() const
{
    return Ptree_util::ptree_to_json_str(metadata_to_ptree());
}
boost::property_tree::ptree Property_base::metadata_to_ptree() const
{
    boost::property_tree::ptree tree;

    //data
    tree.put("name",  name_);
    tree.put("value", value_to_string());
    tree.put("type",  type_);

    //metadata
    tree.put("desc",  desc_);
    
    boost::property_tree::ptree constraints;
    this->put_constraints(&constraints);
    tree.put_child("constraints", constraints);

    return tree;
}

void Property_base::put_constraints(boost::property_tree::ptree* const tree) const
{
    tree->clear();
}

void Property_base::set_constraints_from_tree(const boost::property_tree::ptree& tree)
{

}
