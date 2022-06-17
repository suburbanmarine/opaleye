/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property.hpp"

#include <cstdint>

#include <string>

class ipv6_addr
{
public:

	virtual bool operator==(const ipv6_addr& lhs) const
	{
		return std::equal(addr.begin(), addr.end(), lhs.addr.begin());
	}

	virtual bool operator<(const ipv6_addr& lhs) const
	{
		return std::equal(addr.begin(), addr.end(), lhs.addr.begin(), lhs.addr.end());
	}

	std::string to_string() const;

	std::array<uint8_t, 16> addr;
};

class ipv6_endpoint
{
public:

	virtual bool operator==(const ipv6_endpoint& lhs) const
	{
		return
		 	(host == lhs.host) &&
			(port == lhs.port);
	}

	virtual bool operator<(const ipv6_endpoint& lhs) const
	{
		return
		 	(host < lhs.host) &&
			(port < lhs.port);
	}

	ipv6_addr host;
	uint16_t  port;
};

class Property_ipv6_endpoint : public Property<ipv6_endpoint>
{
public:

	Property_ipv6_endpoint()
	{

	}
	
	bool is_value_valid(const ipv6_endpoint& x) const override
	{
		return false;
	}

protected:
    std::string value_to_string(const ipv6_endpoint& val) const override;
    ipv6_endpoint value_from_string(const std::string& str) const override;
};
