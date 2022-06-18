#include "property/Property_ipv4_endpoint.hpp"

#include <spdlog/fmt/fmt.h>

#include <sstream>

std::string ipv4_addr::to_string() const
{
	std::stringstream ss;
	for(size_t i = 0; i < (addr.size()-1); i++)
	{
		ss << fmt::format("{:d}.", addr[i]);
	}
	ss << fmt::format("{:d}", addr[(addr.size()-1)]);
	return ss.str();
}

std::string ipv4_endpoint::to_string() const
{
	return fmt::format("{:s}:{:d}", host.to_string(), port);
}

std::string Property_ipv4_endpoint::value_to_string(const ipv4_endpoint& val) const
{
	return val.to_string();
}
ipv4_endpoint Property_ipv4_endpoint::value_from_string(const std::string& str) const
{
	return ipv4_endpoint();
}