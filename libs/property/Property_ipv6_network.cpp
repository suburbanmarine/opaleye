#include "property/Property_ipv6_network.hpp"

#include <spdlog/fmt/fmt.h>

#include <sstream>

std::string Property_ipv6_network::value_to_string(const ipv6_network& val) const
{
	std::stringstream ss;
	ss << val.host.to_string() << "/" << val.mask.to_string();
	return ss.str();
}
ipv6_network Property_ipv6_network::value_from_string(const std::string& str) const
{
	return ipv6_network();
}