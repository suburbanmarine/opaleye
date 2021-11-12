#include "config/Opaleye_config.hpp"

#include <iostream>

int main()
{
	app_config config;

	config.make_default();

	std::cout << config.to_xml_string();
	std::cout << "\n";
	std::cout << config.to_json_string();
	std::cout << "\n";

	return 0;
}
