#pragma once

#include <boost/property_tree/ptree_fwd.hpp>

#include <string>

class Ptree_util
{
public:
	static std::string ptree_to_json_str(const boost::property_tree::ptree& tree);
	static std::string ptree_to_xml_str(const boost::property_tree::ptree& tree);

	static std::string ptree_to_json_str(const boost::property_tree::ptree& tree, const bool pretty);
	static std::string ptree_to_xml_str(const boost::property_tree::ptree& tree, const bool pretty);

	static void ptree_to_json_str(const boost::property_tree::ptree& tree, const bool pretty, std::string * const out_str);
	static void ptree_to_xml_str(const boost::property_tree::ptree& tree, const bool pretty, std::string * const out_str);
};
