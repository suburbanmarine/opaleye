#include "property/Property_numeric.hpp"

#include <spdlog/fmt/fmt.h>

#include <cinttypes>

std::string Property_numeric_i8::value_to_string(const int8_t& val) const
{
	return fmt::format("{:d}", int(val));
}
std::string Property_numeric_u8::value_to_string(const uint8_t& val) const
{
	return fmt::format("{:d}", unsigned(val));
}
std::string Property_numeric_i16::value_to_string(const int16_t& val) const
{
	return fmt::format("{:d}", val);
}
std::string Property_numeric_u16::value_to_string(const uint16_t& val) const
{
	return fmt::format("{:d}", val);
}
std::string Property_numeric_i32::value_to_string(const int32_t& val) const
{
	return fmt::format("{:d}", val);
}
std::string Property_numeric_u32::value_to_string(const uint32_t& val) const
{
	return fmt::format("{:d}", val);
}
std::string Property_numeric_i64::value_to_string(const int64_t& val) const
{
	return fmt::format("{:d}", val);
}
std::string Property_numeric_u64::value_to_string(const uint64_t& val) const
{
	return fmt::format("{:d}", val);
}
std::string Property_numeric_float::value_to_string(const float& val) const
{
	return fmt::format("{:.9e}", int(val));
}
std::string Property_numeric_double::value_to_string(const double& val) const
{
	return fmt::format("{:.17e}", int(val));
}

int8_t Property_numeric_i8::value_from_string(const std::string& str) const
{
	return sscanf_helper<int>(str.c_str(), "%d");
}
uint8_t Property_numeric_u8::value_from_string(const std::string& str) const
{
	return sscanf_helper<unsigned>(str.c_str(), "%d");
}
int16_t Property_numeric_i16::value_from_string(const std::string& str) const
{
	return sscanf_helper<int>(str.c_str(), "%d");
}
uint16_t Property_numeric_u16::value_from_string(const std::string& str) const
{
	return sscanf_helper<unsigned>(str.c_str(), "%d");
}
int32_t Property_numeric_i32::value_from_string(const std::string& str) const
{
	return sscanf_helper<int32_t>(str.c_str(), PRId32);
}
uint32_t Property_numeric_u32::value_from_string(const std::string& str) const
{
	return sscanf_helper<uint32_t>(str.c_str(), PRIu32);
}
int64_t Property_numeric_i64::value_from_string(const std::string& str) const
{
	return sscanf_helper<int64_t>(str.c_str(), PRId64);
}
uint64_t Property_numeric_u64::value_from_string(const std::string& str) const
{
	return sscanf_helper<uint64_t>(str.c_str(), PRIu64);
}
float Property_numeric_float::value_from_string(const std::string& str) const
{
	return sscanf_helper<double>(str.c_str(), "%f");
}
double Property_numeric_double::value_from_string(const std::string& str) const
{
	return sscanf_helper<double>(str.c_str(), "%f");
}