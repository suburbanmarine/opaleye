/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "gpio_thread.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

#include <sys/timerfd.h>
#include <unistd.h>

gpio_thread::gpio_thread()
{
	m_timer_fd = -1;
}
gpio_thread::~gpio_thread()
{
	if(m_timer_fd)
	{
		int ret = close(m_timer_fd);
		if(ret != 0)
		{
			//log
		}
	}
}

bool gpio_thread::init()
{
	m_timer_fd = timerfd_create(CLOCK_REALTIME, 0);
	if(m_timer_fd < 0)
	{
		//log
		return false;
	}

	return true;
}

void gpio_thread::work()
{	
	timespec t_now;
	int ret = clock_gettime(CLOCK_REALTIME, &t_now);

	itimerspec alarm_interval;
	alarm_interval.it_interval.tv_sec  = 1; /* Interval for periodic timer */
	alarm_interval.it_interval.tv_nsec = 0;
	alarm_interval.it_value.tv_sec     = t_now.tv_sec + 1; /* Initial expiration */
	alarm_interval.it_value.tv_nsec    = 0;
	ret = timerfd_settime(m_timer_fd, TFD_TIMER_ABSTIME, &alarm_interval, nullptr);
	while( ! is_interrupted() )
	{
		uint64_t exp_ctr = 0;
		int ret = read(m_timer_fd, &exp_ctr, sizeof(exp_ctr));
		if(ret < 0)
		{

		}
		else
		{
			if(ret != 8)
			{

			}
			else
			{
				SPDLOG_INFO("GPIO woke");
			}
		}
	}
}
