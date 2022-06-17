/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property.hpp"
#include "property/Property_ipv4_endpoint.hpp"

#include <cstdint>

class ipv4_network
{
public:

	virtual bool operator==(const ipv4_network& lhs) const
	{
		return
		 	(host == lhs.host) &&
			(mask == lhs.mask);
	}

	virtual bool operator<(const ipv4_network& lhs) const
	{
		return
		 	(host < lhs.host) &&
			(mask < lhs.mask);
	}

	ipv4_addr host;
	ipv4_addr mask;
};

class Property_ipv4_network : public Property<ipv4_network>
{
public:

	Property_ipv4_network()
	{

	}
	
	bool is_value_valid(const ipv4_network& x) const override
	{
		return false;
	}

protected:
    std::string value_to_string(const ipv4_network& val) const override;
    ipv4_network value_from_string(const std::string& str) const override;
};
