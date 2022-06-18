#include "property/Property_ipv6_endpoint.hpp"

#include <spdlog/fmt/fmt.h>

#include <sstream>

std::string ipv6_addr::to_string() const
{
	std::stringstream ss;
	for(size_t i = 0; i < (addr.size()-1); i++)
	{
		ss << fmt::format("{:02x}:", addr[i]);
	}
	ss << fmt::format("{:02x}", addr[(addr.size()-1)]);
	return ss.str();
}

std::string ipv6_endpoint::to_string() const
{
	return fmt::format("{:s}:{:d}", host.to_string(), port);
}

std::string Property_ipv6_endpoint::value_to_string(const ipv6_endpoint& val) const
{
	return val.to_string();
}
ipv6_endpoint Property_ipv6_endpoint::value_from_string(const std::string& str) const
{
	return ipv6_endpoint();
}