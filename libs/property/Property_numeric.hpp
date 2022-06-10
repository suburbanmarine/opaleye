/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "Property.hpp"

#include <limits>

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
	
	bool is_value_valid(const T& x) const override
	{
		return ( (min() <= x) && (x <= max()) );
	}

protected:
	const T min_;
	const T max_;
};

class Property_numeric_i8 : Property_numeric<int8_t>
{

};
class Property_numeric_u8 : Property_numeric<uint8_t>
{
};
class Property_numeric_i16 : Property_numeric<int16_t>
{
};
class Property_numeric_u16 : Property_numeric<uint16_t>
{
};
class Property_numeric_i32 : Property_numeric<int32_t>
{
};
class Property_numeric_u32 : Property_numeric<uint32_t>
{
};
class Property_numeric_i64 : Property_numeric<int64_t>
{
};
class Property_numeric_u64 : Property_numeric<uint64_t>
{
};
class Property_numeric_float : Property_numeric<float>
{

};
class Property_numeric_double : Property_numeric<double>
{

};

template<typename T>
class Property_numeric_factory
{
	static std::shared_ptr<Property_numeric<T>> create(const T& val, std::string, std::string);
};

