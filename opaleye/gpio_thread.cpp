/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "gpio_thread.hpp"
#include "opaleye-util/chrono_util.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

#include <sys/timerfd.h>
#include <unistd.h>

//for xavier
#define TEGRA194_MAIN_GPIO_PORT_A 0
#define TEGRA194_MAIN_GPIO_PORT_B 1
#define TEGRA194_MAIN_GPIO_PORT_C 2
#define TEGRA194_MAIN_GPIO_PORT_D 3
#define TEGRA194_MAIN_GPIO_PORT_E 4
#define TEGRA194_MAIN_GPIO_PORT_F 5
#define TEGRA194_MAIN_GPIO_PORT_G 6
#define TEGRA194_MAIN_GPIO_PORT_H 7
#define TEGRA194_MAIN_GPIO_PORT_I 8
#define TEGRA194_MAIN_GPIO_PORT_J 9
#define TEGRA194_MAIN_GPIO_PORT_K 10
#define TEGRA194_MAIN_GPIO_PORT_L 11
#define TEGRA194_MAIN_GPIO_PORT_M 12
#define TEGRA194_MAIN_GPIO_PORT_N 13
#define TEGRA194_MAIN_GPIO_PORT_O 14
#define TEGRA194_MAIN_GPIO_PORT_P 15
#define TEGRA194_MAIN_GPIO_PORT_Q 16
#define TEGRA194_MAIN_GPIO_PORT_R 17
#define TEGRA194_MAIN_GPIO_PORT_S 18
#define TEGRA194_MAIN_GPIO_PORT_T 19
#define TEGRA194_MAIN_GPIO_PORT_U 20
#define TEGRA194_MAIN_GPIO_PORT_V 21
#define TEGRA194_MAIN_GPIO_PORT_W 22
#define TEGRA194_MAIN_GPIO_PORT_X 23
#define TEGRA194_MAIN_GPIO_PORT_Y 24
#define TEGRA194_MAIN_GPIO_PORT_Z 25
#define TEGRA194_MAIN_GPIO_PORT_FF 26
#define TEGRA194_MAIN_GPIO_PORT_GG 27

#define TEGRA194_MAIN_GPIO(port, offset) \
        ((TEGRA194_MAIN_GPIO_PORT_##port * 8) + offset)

gpio_thread::gpio_thread()
{
	m_timer_fd = -1;
	m_gpio_chip0 = nullptr;
	m_line       = nullptr;
}
gpio_thread::~gpio_thread()
{
	close();
}

bool gpio_thread::close()
{
	bool ret = true;
	if(m_timer_fd)
	{
		int ret = ::close(m_timer_fd);
		if(ret != 0)
		{
			SPDLOG_ERROR("close(m_timer_fd) failed");
			ret = false;
		}
	}

	if( m_line )
	{
		gpiod_line_release(m_line);
		m_line = nullptr;
	}

	if( m_gpio_chip0 )
	{
		gpiod_chip_close(m_gpio_chip0);
		m_gpio_chip0 = nullptr;
	}

	return ret;
}

bool gpio_thread::init()
{
	m_timer_fd = timerfd_create(CLOCK_REALTIME, 0);
	if(m_timer_fd < 0)
	{
		//log
		close();
		return false;
	}

	m_gpio_chip0 = gpiod_chip_open("/dev/gpiochip0"); // tegra-gpio, base 288
	// m_gpio_chip1 = gpiod_chip_open("/dev/gpiochip1"); // tegra-gpio-aon, base 248
	// m_gpio_chip2 = gpiod_chip_open("/dev/gpiochip2"); // max77620-gpio, base 246
	if(m_gpio_chip0 == nullptr)
	{
		SPDLOG_ERROR("gpiod_chip_open failed");
		close();
		return false;
	}

	//CAM0_PWDN / cam0 GPIO0 - xavier GPIO03_P04
	//CAM1_PWDN / cam1 GPIO0 - xavier GPIO03_P05
	//CAM0_MCLK / cam0 GPIO1 - xavier GPIO03_P00
	//CAM1_MCLK / cam1 GPIO1 - xavier GPIO03_P01
	m_line = gpiod_chip_get_line(m_gpio_chip0, TEGRA194_MAIN_GPIO_PORT_P*8+0);
	if(m_line == nullptr)
	{
		SPDLOG_ERROR("gpiod_chip_get_line failed");
		close();
		return false;
	}

	int ret = gpiod_line_request_output(m_line, "opaleye-GPIO03_P00", 0);
	if(ret != 0)
	{
		SPDLOG_ERROR("gpiod_line_request_output failed");
		close();
		return false;
	}

	set(false);

	return true;
}

bool gpio_thread::set(const bool val)
{
	return 0 == gpiod_line_set_value(m_line, val);
}
void gpio_thread::work()
{	

	while( ! is_interrupted() )
	{
		// next top of second
		timespec now;
		int ret = clock_gettime(CLOCK_REALTIME, &now);
		now.tv_sec  += 1;
		now.tv_nsec  = 0;

		//wait
		ret = 0;
		do
		{
			ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &now, NULL);
		} while( (ret != 0) && (errno == EINTR));


		//fire
		if(ret == 0)
		{
			set(true);

			timespec delay;
			delay.tv_sec  = 0;
			delay.tv_nsec  = 10*1000*1000;
			do
			{
				ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &delay, &delay);
			} while( (ret != 0) && (errno == EINTR));

			set(false);
		}
	}

#if 0
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
#endif
}