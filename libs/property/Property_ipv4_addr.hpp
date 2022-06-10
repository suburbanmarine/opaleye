/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

class ipv4_addr
{

};

class Property_ipv4_addr : Property<ipv4_addr>
{
public:

	Property_ipv4_addr()
	{

	}
	
	bool is_value_valid() override
	{
		return false;
	}

protected:

};
