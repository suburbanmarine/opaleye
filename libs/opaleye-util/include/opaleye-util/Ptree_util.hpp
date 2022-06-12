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

	static boost::property_tree::ptree json_str_to_ptree(const std::string& str);
	static boost::property_tree::ptree xml_str_to_ptree(const std::string& str);

	static void json_str_to_ptree(const std::string& str, boost::property_tree::ptree* const out_tree);
	static void xml_str_to_ptree(const std::string& str, boost::property_tree::ptree* const out_tree);
};
