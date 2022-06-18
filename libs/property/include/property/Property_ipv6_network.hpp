/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property.hpp"
#include "property/Property_ipv6_endpoint.hpp"

#include <cstdint>

#include <string>

class ipv6_network
{
public:

	// ipv6_network()
	// {
		
	// }

	virtual bool operator==(const ipv6_network& lhs) const
	{
		return
		 	(host == lhs.host) &&
			(mask == lhs.mask);
	}

	virtual bool operator<(const ipv6_network& lhs) const
	{
		return
		 	(host < lhs.host) &&
			(mask < lhs.mask);
	}

	std::string to_string() const;

	ipv6_addr host;
	ipv6_addr mask;
};

class Property_ipv6_network : public Property<ipv6_network>
{
public:

	Property_ipv6_network()
	{

	}
	
	bool is_value_valid(const ipv6_network& x) const override
	{
		// essentially, any bit pattern in x is valid
		// even 0.0.0.0 has several valid meanings depending on context
		return true;
	}

protected:
    std::string value_to_string(const ipv6_network& val) const override;
    ipv6_network value_from_string(const std::string& str) const override;
};
