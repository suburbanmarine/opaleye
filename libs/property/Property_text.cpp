#include "Property_text.hpp"

#include <spdlog/fmt/fmt.h>

std::string Property_text::value_to_string(const std::string& val) const
{
	return val;
}

void Property_text::value_from_string(const std::string& str)
{
	val_ = str;
}
