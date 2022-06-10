/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "Property_base.hpp"

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

	virtual std::string to_string() const
	{
	    std::stringstream ss;
		ss << val_;
		return ss.str();  
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

protected:
	T val_;
};
