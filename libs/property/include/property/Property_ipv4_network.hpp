/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "property/Property.hpp"

#include <cstdint>

class ipv4_network
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

	ipv4_host host;
	ipv4_host mask;
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
