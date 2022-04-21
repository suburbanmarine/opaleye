#include "Opaleye_gpio_mod_ctrl.hpp"

#include "opaleye-util/chrono_util.hpp"
#include "opaleye-util/errno_util.hpp"

#include <boost/filesystem.hpp>

#include <spdlog/spdlog.h>

#include <cstdio>
#include <unistd.h>
    #include <sys/stat.h>
       #include <fcntl.h>

Opaleye_gpio_mod_ctrl::Opaleye_gpio_mod_ctrl()
{
	m_fd_enable = -1;
	m_fd_config = -1;
}
Opaleye_gpio_mod_ctrl::~Opaleye_gpio_mod_ctrl()
{

}

bool Opaleye_gpio_mod_ctrl::is_loaded()
{
	return boost::filesystem::exists("/sys/kernel/opaleye");
}

bool Opaleye_gpio_mod_ctrl::open()
{
	m_fd_enable = ::open("/sys/kernel/opaleye/enable", O_RDWR);
	if(m_fd_enable == -1)
	{
		close();
		return false;
	}
	m_fd_config = ::open("/sys/kernel/opaleye/timer_settings", O_RDWR);
	if(m_fd_config == -1)
	{
		close();
		return false;
	}

	return true;
}
bool Opaleye_gpio_mod_ctrl::close()
{
	errno_util err;

	bool ret = true;

	if(m_fd_enable != -1)
	{
		int ret = ::close(m_fd_enable);
		if(ret != 0)
		{
			SPDLOG_ERROR("Error closing m_fd_enable: {:s}", err.to_str());
			ret = false;
		}

		m_fd_enable = -1;
	}
	if(m_fd_config != -1)
	{
		int ret = ::close(m_fd_config);
		if(ret != 0)
		{
			SPDLOG_ERROR("Error closing m_fd_config: {:s}", err.to_str());
			ret = false;
		}

		m_fd_config = -1;
	}

	return ret;
}

bool Opaleye_gpio_mod_ctrl::enable()
{
	int ret = dprintf(m_fd_enable, "1\n");

	return ret > 0;
}
bool Opaleye_gpio_mod_ctrl::disable()
{
	int ret = dprintf(m_fd_enable, "0\n");

	return ret > 0;
}
bool Opaleye_gpio_mod_ctrl::set_enable(const bool en)
{
	if(en)
	{
		return enable();
	}
	return disable();
}
bool Opaleye_gpio_mod_ctrl::configure(const std::chrono::nanoseconds& t0, const std::chrono::nanoseconds& period, const std::chrono::nanoseconds& width)
{
	timespec t0_ts     = chrono_to_timespec(t0);
	timespec period_ts = chrono_to_timespec(period);
	timespec width_ts  = chrono_to_timespec(width);
	int ret = dprintf(m_fd_config, "%ld.%09ld,%ld.%09ld,%ld.%09ld\n",
		(long)t0_ts.tv_sec,     (long)t0_ts.tv_nsec,
		(long)period_ts.tv_sec, (long)period_ts.tv_nsec,
		(long)width_ts.tv_sec,  (long)width_ts.tv_nsec
	);

	return ret > 0;
}
