#include "opaleye-util/Ptree_util.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <sstream>

std::string Ptree_util::ptree_to_json_str(const boost::property_tree::ptree& tree)
{
	return ptree_to_json_str(tree, true);
}
std::string Ptree_util::ptree_to_xml_str(const boost::property_tree::ptree& tree)
{
	return ptree_to_xml_str(tree, true);
}

std::string Ptree_util::ptree_to_json_str(const boost::property_tree::ptree& tree, const bool pretty)
{
	std::string str;
	ptree_to_json_str(tree, pretty, &str);
	return str;
}
std::string Ptree_util::ptree_to_xml_str(const boost::property_tree::ptree& tree, const bool pretty)
{
	std::string str;
	ptree_to_xml_str(tree, pretty, &str);
	return str;
}

void Ptree_util::ptree_to_json_str(const boost::property_tree::ptree& tree, const bool pretty, std::string * const out_str)
{
	std::stringstream ss;
	boost::property_tree::write_json(ss, tree, pretty);
	*out_str = std::move(ss).str();
}
void Ptree_util::ptree_to_xml_str(const boost::property_tree::ptree& tree, const bool pretty, std::string * const out_str)
{
	std::stringstream ss;
	boost::property_tree::write_xml(ss, tree, pretty);
	*out_str = std::move(ss).str();
}