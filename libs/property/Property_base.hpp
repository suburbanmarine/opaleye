#pragma once

#include <string>
#include <memory>
#include <map>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

template<typename T>
class Property;

class Property_base
{
public:
	Property_base()
	{

	}
	virtual ~Property_base()
	{

	}

	virtual bool is_value_valid() const = 0;

	virtual std::string to_string() const = 0;
	virtual void from_string(const std::string& str) const = 0;

	virtual rapidjson::Value to_json_value() const = 0;
	virtual void from_json_value(const rapidjson::Value& val) const = 0;

	const std::string& get_name() const
	{
		return name_;
	}
	const std::string& get_desc() const
	{
		return desc_;
	}
protected:
	std::string name_;
	std::string desc_;
};

class PropertyCollection
{
public:

	virtual std::string to_string() const = 0;
	virtual std::string to_json_obj() const
	{
		rapidjson::Document doc;

		std::shared_ptr<Property_base> p;
		for(const auto& prop : property_storage_)
		{
			// doc.AddMember(p->get_name(), p->to_json_value(), doc.GetAllocator());
		}
	}

	virtual void from_json_obj(const std::string& msg) const
	{
		std::shared_ptr<Property_base> p;
		for(const auto& prop : property_storage_)
		{
			// doc.AddMember(p->get_name(), p->get_json_val(), doc.GetAllocator());
		}
	}

	virtual void from_json_obj(const rapidjson::Document& doc) const
	{

	}

	std::map<std::string, std::shared_ptr<Property_base>> property_storage_;
};
