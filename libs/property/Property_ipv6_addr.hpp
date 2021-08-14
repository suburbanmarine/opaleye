class ipv6_addr
{

};

class Property_ipv6_addr : Property<ipv6_addr>
{
public:

	Property_ipv6_addr()
	{

	}
	
	bool is_value_valid() override
	{
		return false;
	}

protected:

};
