#include "property/Property_ipv4_endpoint.hpp"

#include <spdlog/fmt/fmt.h>

#include <sstream>

#include <cstdio>

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
	std::array<int, 5> temp;
	int ret = sscanf(str.c_str(), "%d.%d.%d.%d:%d", 
		temp.data() + 0,
		temp.data() + 1,
		temp.data() + 2,
		temp.data() + 3,
		temp.data() + 4
	);
	if(ret < 0)
	{
		throw std::runtime_error("Could not parse ipv4_endpoint, sscanf error");
	}
	if(ret != 5)
	{
		throw std::invalid_argument("Could not parse ipv4_endpoint");
	}

	ipv4_endpoint e;
	for(int i = 0; i < 4; i++)
	{
		if( (temp[i] < 0) || (temp[i] > 255) )
		{
			throw std::invalid_argument("ip fragment must be in [0, 255]");
		}

		e.host.addr[i] = temp[i];
	}

	if( (temp[5] < 0) || (temp[5] > 65535) )
	{
		throw std::invalid_argument("port must be in [0, 65535]");
	}
	e.port = temp[5];
	
	return e;
}