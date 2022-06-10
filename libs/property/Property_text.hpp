/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/


#include "Property.hpp"

#include <string>

class Property_text : public Property<std::string>
{
public:

    bool is_value_valid(const std::string& x) const override
    {
        return true;
    }

protected:

};
