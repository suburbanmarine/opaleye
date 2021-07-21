#pragma once

#include <map>
#include <mutex>
#include <string>

class linux_thermal_zone
{
public:

	linux_thermal_zone();
	virtual ~linux_thermal_zone();

	bool sample();

	void get_temps(std::map<std::string, double>* const out_temps)
	{
	    std::unique_lock<std::mutex> lock(m_temps_mutex);
		*out_temps = m_temps;
	}

protected:
	bool read_file_to_string(const char* path, std::string* const out_str);

	std::mutex m_temps_mutex;
	std::map<std::string, double> m_temps;
};
