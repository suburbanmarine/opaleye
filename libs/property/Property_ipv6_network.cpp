#include "property/Property_ipv6_network.hpp"

#include <spdlog/fmt/fmt.h>

#include <sstream>

std::string ipv6_network::to_string() const
{
	return fmt::format("{:s}/{:s}", host.to_string(), mask.to_string());
}

std::string Property_ipv6_network::value_to_string(const ipv6_network& val) const
{
	return val.to_string();
}
ipv6_network Property_ipv6_network::value_from_string(const std::string& str) const
{
	return ipv6_network();
}