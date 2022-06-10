/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "Property.hpp"

#include <string>
#include <set>

template <typename T>
class Property_enum : public Property<T>
{
public:

	bool is_value_valid(const T& x) const override
	{
		return valid_values_.find(x) != valid_values_.end();
	}

	virtual const std::set<Property<T>>& valid_value_set() const
	{
		return valid_values_;
	}

	virtual const std::set<Property<T>>& add_valid_value(const Property<T>& val) const
	{
		return valid_values_.insert(val);
	}

protected:
	std::set<Property<T>> valid_values_;
};

class Property_enum_int : public Property_enum<int>
{

};
class Property_enum_char : public Property_enum<char>
{

};
// class Property_enum_void : public Property_enum<void>
// {

// };


