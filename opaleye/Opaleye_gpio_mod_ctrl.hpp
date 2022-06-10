#pragma once

#include <chrono>

class Opaleye_gpio_mod_ctrl
{
public:

	Opaleye_gpio_mod_ctrl();
	~Opaleye_gpio_mod_ctrl();

	static bool is_loaded();

	bool open();
	bool close();

	bool enable();
	bool disable();

	bool set_enable(const bool en);
	bool configure(const std::chrono::nanoseconds& t0, const std::chrono::nanoseconds& period, const std::chrono::nanoseconds& width);

protected:
	int m_fd_enable;
	int m_fd_config;
};
