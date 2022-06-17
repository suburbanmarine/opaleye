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

boost::property_tree::ptree Ptree_util::json_str_to_ptree(const std::string& str)
{
	boost::property_tree::ptree tree;

	std::stringstream ss;
	ss << str;
	boost::property_tree::read_json(ss, tree);

	return tree;
}
boost::property_tree::ptree Ptree_util::xml_str_to_ptree(const std::string& str)
{
	boost::property_tree::ptree tree;

	std::stringstream ss;
	ss << str;
	boost::property_tree::read_xml(ss, tree);

	return tree;
}

void Ptree_util::json_str_to_ptree(const std::string& str, boost::property_tree::ptree* const out_tree)
{
	std::stringstream ss;
	ss << str;
	boost::property_tree::read_json(ss, *out_tree);
}
void Ptree_util::xml_str_to_ptree(const std::string& str, boost::property_tree::ptree* const out_tree)
{
	std::stringstream ss;
	ss << str;
	boost::property_tree::read_xml(ss, *out_tree);
}