/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <string>
#include <memory>
#include <map>

#include <boost/property_tree/ptree_fwd.hpp>

template<typename T>
class Property;

class Property_factory;

class Property_base
{
public:
	friend Property_factory;

	Property_base()
	{

	}
	virtual ~Property_base()
	{

	}

	virtual bool is_value_valid() const = 0;

	virtual std::string to_json() const;
	virtual boost::property_tree::ptree to_ptree() const;

	virtual std::string metadata_to_json() const;
	virtual boost::property_tree::ptree metadata_to_ptree() const;

	const std::string& name() const
	{
		return name_;
	}
	const std::string& desc() const
	{
		return desc_;
	}
protected:

	virtual std::string value_to_string() const = 0;
	virtual void set_value_from_string(const std::string& str) = 0;

	virtual void put_constraints(boost::property_tree::ptree* const tree) const;

	virtual void set_constraints_from_tree(const boost::property_tree::ptree& tree);

	std::string name_;
	std::string desc_;
	std::string type_;
};

/*
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

		return
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
*/