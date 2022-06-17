/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "property/Property_base.hpp"

#include <boost/container_hash/hash.hpp>

#include <sstream>
#include <string>

template<typename T>
class Property : public Property_base
{
public:

	Property()
	{

	}
	virtual ~Property()
	{

	}

	T& value()
	{
		return val_;
	}
	const T& value() const
	{
		return val_;
	}

	virtual bool is_value_valid(const T& x) const = 0;

	virtual bool is_value_valid() const
	{
		return is_value_valid(val_);
	}

	virtual bool operator==(const Property<T>& lhs) const
	{
		return
		 	(val_   == lhs.val_)  &&
			(name_  == lhs.name_) &&
			(desc_  == lhs.desc_);
	}

	virtual bool operator<(const Property<T>& lhs) const
	{
		return
		 	(val_   < lhs.val_)  &&
			(name_  < lhs.name_) &&
			(desc_  < lhs.desc_);
	}

	std::size_t hash_value(Property<T>& prop)
	{
		std::size_t seed = 0;
			
		boost::hash_combine(seed, boost::hash<T>(val_));
		boost::hash_combine(seed, boost::hash<std::string>(name_));
		boost::hash_combine(seed, boost::hash<std::string>(desc_));

		return seed;
	}

protected:

	virtual std::string value_to_string(const T& val) const = 0;
	virtual T value_from_string(const std::string& str) const = 0;

	void set_value_from_string(const std::string& str) override
	{
		val_ = value_from_string(str);
	}

	std::string value_to_string() const override
	{
		return value_to_string(val_);
	}

	T val_;
};

template<typename T>
std::size_t hash_value(const Property<T>& prop)
{
    return prop.hash_value();
}
