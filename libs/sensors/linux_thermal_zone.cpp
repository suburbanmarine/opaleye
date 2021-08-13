/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "linux_thermal_zone.hpp"

#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <regex>

linux_thermal_zone::linux_thermal_zone()
{

}
linux_thermal_zone::~linux_thermal_zone()
{
	
}

bool linux_thermal_zone::sample()
{
	boost::filesystem::path acpi_zone = "/sys/class/thermal";

	std::regex thermal_zone_regex("thermal_zone\\d+");

	std::string type_data;
	std::string temp_data;
	std::map<std::string, double> new_temps;

    for(auto& dir_entry : boost::make_iterator_range(boost::filesystem::directory_iterator(acpi_zone), {}))
    {
        boost::filesystem::file_status dir_entry_status = boost::filesystem::status(dir_entry);
        if(boost::filesystem::is_directory(dir_entry_status))
        {
        	std::string dir_name = dir_entry.path().filename().string();
        	if(std::regex_match(dir_name, thermal_zone_regex, std::regex_constants::match_default))
        	{
				boost::filesystem::path type_path = dir_entry.path() / "type";
				boost::filesystem::path temp_path = dir_entry.path() / "temp";

				type_data.clear();
				bool ret = read_file_to_string(type_path.string().c_str(), &type_data);
				if(!ret)
				{
					return false;	
				}

				temp_data.clear();
				ret = read_file_to_string(temp_path.string().c_str(), &temp_data);
				if(!ret)
				{
					return false;
				}

				if(type_data.back() == '\n')
				{
					type_data.pop_back();
				}

				double new_temp = 0.0;
				if(sscanf(temp_data.c_str(), "%lf", &new_temp) != 1)
				{
					return false;
				}

				new_temp /= 1000.0;

				new_temps.insert(std::make_pair(type_data, new_temp));
        	}
        }
    }

	{
	    std::unique_lock<std::mutex> lock(m_temps_mutex);
		m_temps = new_temps;
	}

    return true;
}

bool linux_thermal_zone::read_file_to_string(const char* path, std::string* const out_str)
{
	boost::filesystem::file_status path_status = boost::filesystem::status(path);
	if( ! boost::filesystem::is_regular_file(path_status) )
	{
		return false;
	}

	const uintmax_t file_len = boost::filesystem::file_size(path);
	if( file_len > 8192ULL )
	{
		return false;
	}

	out_str->reserve(file_len);

	FILE* f = fopen(path, "rb");
	if(!f)
	{
		return false;
	}

	int eof    = 0;
	int f_error = 0;

	std::array<char, 128> buf;
	size_t num_read = 0;
	do
	{
		num_read = fread(buf.data(), 1, buf.size(), f);

		if(num_read == 0)
		{
			int eof    = feof(f);
			int f_error = ferror(f);
		}
		else
		{
			out_str->insert(out_str->end(), buf.data(), buf.data() + num_read);
		}

	} while(num_read > 0);

	int close_ret = fclose(f);
	f = nullptr;

	//these files don't set eof correctly
	// return (close_ret == 0) && (eof) && (f_error == 0);
	return (close_ret == 0) && (f_error == 0);
}