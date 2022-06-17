#include "property/Property_ipv4_network.hpp"

std::string Property_ipv4_network::value_to_string(const ipv4_network& val) const
{
	std::stringstream ss;
	ss << val.host.to_string() << "/" << val.mask.to_string();
	return ss.str();
}
ipv4_network Property_ipv4_network::value_from_string(const std::string& str) const
{
	return ipv4_network();
}
