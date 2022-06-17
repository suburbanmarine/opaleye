/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2022 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property_base.hpp"

class Property_factory
{
    static std::shared_ptr<Property_base> from_json(const std::string& tree);
    static std::shared_ptr<Property_base> from_ptree(const boost::property_tree::ptree& str);
};
