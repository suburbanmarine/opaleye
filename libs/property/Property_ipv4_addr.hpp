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
