#pragma once

#include "Property_base.hpp"

template<typename T>
class Property : Property_base
{
public:

	Property()
	{

	}
	virtual ~Property()
	{

	}

	void set_value(const T& x)
	{
		val_ = x;
	}
	T& get_value()
	{
		return val_;
	}
	const T& get_value() const
	{
		return val_;
	}

protected:
	T val_;
};
