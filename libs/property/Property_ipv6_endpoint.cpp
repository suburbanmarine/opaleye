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
