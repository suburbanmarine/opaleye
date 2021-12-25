#pragma once

#include <boost/property_tree/ptree_fwd.hpp>

#include <string>

class Ptree_util
{
public:
	static std::string ptree_to_json_str(const boost::property_tree::ptree& tree);
	static std::string ptree_to_xml_str(const boost::property_tree::ptree& tree);
};
