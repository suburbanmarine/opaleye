#include "property/Property_ipv4_network.hpp"

#include <spdlog/fmt/fmt.h>

std::string ipv4_network::to_string() const
{
	return fmt::format("{:s}/{:s}", host.to_string(), mask.to_string());
}

std::string Property_ipv4_network::value_to_string(const ipv4_network& val) const
{
	return val.to_string();
}
ipv4_network Property_ipv4_network::value_from_string(const std::string& str) const
{
	return ipv4_network();
}
