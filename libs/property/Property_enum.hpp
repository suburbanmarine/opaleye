
// a enum of names, that has an int to allow 
class Property_enum_value
{
public:

	Property_enum_value(const int val, const std::string& name) : 
		val_(val),
		name_(name)
		{

		}

	virtual ~Property_enum_value()
	{

	}

	virtual std::string to_string() const;

	//add int cast
	//add string cast

protected:
	const int val_;
	const std::string name_;
};

class Property_enum : Property< Property_enum_value >
{
public:

	virtual const std::set<Property_enum_value>& valid_value_list() const = 0;
	virtual bool is_valid_value(const Property_enum_value& x) const
	{
		return valid_values.find(x) != valid_values.end();
	}

protected:
	std::set<Property_enum_value> valid_values;
};
