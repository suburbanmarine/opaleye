/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property.hpp"

#include <cstdint>

#include <string>

class ipv4_addr
{
public:

	virtual bool operator==(const ipv4_addr& lhs) const
	{
		return std::equal(addr.begin(), addr.end(), lhs.addr.begin());
	}

	virtual bool operator<(const ipv4_addr& lhs) const
	{
		return std::equal(addr.begin(), addr.end(), lhs.addr.begin(), lhs.addr.end());
	}

	std::string to_string() const;

	std::array<uint8_t, 4> addr;
};

class ipv4_endpoint
{
public:

	virtual bool operator==(const ipv4_endpoint& lhs) const
	{
		return
		 	(host == lhs.host) &&
			(port == lhs.port);
	}

	virtual bool operator<(const ipv4_endpoint& lhs) const
	{
		return
		 	(host < lhs.host) &&
			(port < lhs.port);
	}

	ipv4_addr host;
	uint16_t  port;
};

class Property_ipv4_endpoint : public Property<ipv4_endpoint>
{
public:

	Property_ipv4_endpoint()
	{

	}
	
	bool is_value_valid(const ipv4_endpoint& x) const override
	{
		return false;
	}

protected:
    std::string value_to_string(const ipv4_endpoint& val) const override;
    ipv4_endpoint value_from_string(const std::string& str) const override;
};
