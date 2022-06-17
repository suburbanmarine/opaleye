/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property.hpp"

#include <boost/property_tree/ptree.hpp>

#include <string>
#include <set>
#include <unordered_set>
#include <map>

template <typename T>
class Property_enum : public Property<T>
{
public:

	bool is_value_valid(const T& x) const override
	{
		return valid_values_.find(x) != valid_values_.end();
	}

	virtual const std::set<Property<T>>& valid_prop_set() const
	{
		return valid_props_;
	}

	virtual const std::set<Property<T>>& add_valid_value(const Property<T>& prop) const
	{
		//check if name is used
		//check if val  is used
		//if either is used, fail
		//this helps protect against user adding props with same value
		if(
			(valid_names_.find(prop.name())   != valid_names_.end()) || 
			(valid_values_.find(prop.value()) != valid_values_.end())
			)
		{
			//collision
			return false;
		}		

		// new property
		valid_names_.insert(prop.name());
		valid_values_.insert(prop.value());
		valid_props_.insert(std::make_pair(prop.name(), prop));
		return true;
	}

protected:

	void put_constraints(boost::property_tree::ptree* const tree) const override
	{
		boost::property_tree::ptree values;

		for(const auto& v : valid_values_)
		{
			boost::property_tree::ptree val;
			val.put("", v);

			values.push_back(std::make_pair("", val));
		}

		//array of name:value
		tree->put("set", values);
	}

	std::unordered_set<std::string> valid_names_;
	std::unordered_set<T>           valid_values_;

	//name->prop map
	std::map<std::string, Property<T>> valid_props_;
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


