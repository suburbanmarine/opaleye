/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property.hpp"

#include <boost/property_tree/ptree.hpp>

#include <limits>
#include <stdexcept>

template<typename T>
class Property_numeric : public Property<T>
{
public:

	Property_numeric() :
		min_(std::numeric_limits<T>::min()),
		max_(std::numeric_limits<T>::max())
	{

	}

	Property_numeric(const T& min, const T& max) :
		min_(min),
		max_(max)
	{

	}

	const T& min() const
	{
		return min_;
	}
	const T& max() const
	{
		return max_;
	}
	
	using Property<T>::is_value_valid;
	bool is_value_valid(const T& x) const override
	{
		return ( (min() <= x) && (x <= max()) );
	}

	virtual bool operator==(const Property_numeric<T>& lhs) const
	{
		return Property<T>::operator==(lhs) &&
		 	(min_ == lhs.min_)  &&
			(max_ == lhs.max_);
	}

	virtual bool operator<(const Property_numeric<T>& lhs) const
	{
		return Property<T>::operator<(lhs) &&
		 	(min_ < lhs.min_)  &&
			(max_ < lhs.max_);
	}

protected:

	void put_constraints(boost::property_tree::ptree* const tree) const override
	{
		if(min_ != std::numeric_limits<T>::min())
		{
    		tree->put("min", this->value_to_string(min_));
		}
		if(max_ != std::numeric_limits<T>::max())
		{
    		tree->put("max", this->value_to_string(max_));
		}
	}

	void set_constraints_from_tree(const boost::property_tree::ptree& tree) override
	{
		if(tree.count("min"))
		{
			min_ = this->value_from_string(tree.get<std::string>("min"));
		}
		else
		{
			min_ = std::numeric_limits<T>::min();
		}

		if(tree.count("max"))
		{
			max_ = this->value_from_string(tree.get<std::string>("max"));
		}
		else
		{
			max_ = std::numeric_limits<T>::max();
		}
	}

	template< typename U>
	static U sscanf_helper(const char* str, const char* fmt)
	{
		U temp;
		int ret = sscanf(str, fmt, &temp);
		if(ret < 0)
		{
			throw std::runtime_error("sscanf failed");
		}
		if(ret != 1)
		{
			throw std::domain_error("sscanf failed");
		}

		return temp;
	}

	T min_;
	T max_;
};

class Property_numeric_i8 : public Property_numeric<int8_t>
{
public:
	Property_numeric_i8()
	{
		this->type_ = "i8";
	}
protected:
    std::string value_to_string(const int8_t& val) const override;
    int8_t value_from_string(const std::string& str) const override;
};
class Property_numeric_u8 : public Property_numeric<uint8_t>
{
public:
	Property_numeric_u8()
	{
		this->type_ = "u8";
	}
protected:
    std::string value_to_string(const uint8_t& val) const override;
    uint8_t value_from_string(const std::string& str) const override;
};
class Property_numeric_i16 : public Property_numeric<int16_t>
{
public:
	Property_numeric_i16()
	{
		this->type_ = "i16";
	}
protected:
	std::string value_to_string(const int16_t& val) const override;
    int16_t value_from_string(const std::string& str) const override;
};
class Property_numeric_u16 : public Property_numeric<uint16_t>
{
public:
	Property_numeric_u16()
	{
		this->type_ = "u16";
	}
protected:
	std::string value_to_string(const uint16_t& val) const override;
    uint16_t value_from_string(const std::string& str) const override;
};
class Property_numeric_i32 : public Property_numeric<int32_t>
{
public:
	Property_numeric_i32()
	{
		this->type_ = "i32";
	}
protected:
	std::string value_to_string(const int32_t& val) const override;
    int32_t value_from_string(const std::string& str) const override;
};
class Property_numeric_u32 : public Property_numeric<uint32_t>
{
public:
	Property_numeric_u32()
	{
		this->type_ = "u32";
	}
protected:
	std::string value_to_string(const uint32_t& val) const override;
    uint32_t value_from_string(const std::string& str) const override;
};
class Property_numeric_i64 : public Property_numeric<int64_t>
{
public:
	Property_numeric_i64()
	{
		this->type_ = "i64";
	}
protected:
	std::string value_to_string(const int64_t& val) const override;
    int64_t value_from_string(const std::string& str) const override;
};
class Property_numeric_u64 : public Property_numeric<uint64_t>
{
public:
	Property_numeric_u64()
	{
		this->type_ = "u64";
	}
protected:
	std::string value_to_string(const uint64_t& val) const override;
    uint64_t value_from_string(const std::string& str) const override;
};
class Property_numeric_float : public Property_numeric<float>
{
public:
	Property_numeric_float()
	{
		this->type_ = "f32";
	}
protected:
    std::string value_to_string(const float& val) const override;
    float value_from_string(const std::string& str) const override;
};
class Property_numeric_double : public Property_numeric<double>
{
public:
	Property_numeric_double()
	{
		this->type_ = "f64";
	}
protected:
    std::string value_to_string(const double& val) const override;
    double value_from_string(const std::string& str) const override;
};
