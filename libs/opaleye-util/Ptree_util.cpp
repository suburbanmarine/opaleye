#include "opaleye-util/Ptree_util.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <sstream>

std::string Ptree_util::ptree_to_json_str(const boost::property_tree::ptree& tree)
{
	std::stringstream ss;
	boost::property_tree::write_json(ss, tree);
	return ss.str();
}
std::string Ptree_util::ptree_to_xml_str(const boost::property_tree::ptree& tree)
{
	std::stringstream ss;
	boost::property_tree::write_xml(ss, tree);
	return ss.str();
}
