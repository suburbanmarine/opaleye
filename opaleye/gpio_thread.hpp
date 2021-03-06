/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "opaleye-util/thread_base.hpp"

#include <gpiod.h>

#include <mutex>

class gpio_thread : public thread_base
{
public:

	gpio_thread();
	~gpio_thread() override;

	bool init();
	bool close();
	bool set(const bool val);

	void work() override;

protected:
	int m_timer_fd;

	gpiod_chip* m_gpio_chip0;
	gpiod_line* m_line;
};