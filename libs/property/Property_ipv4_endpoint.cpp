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
