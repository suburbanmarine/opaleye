/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2022 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "opaleye-util/chrono_util.hpp"

timeval chrono_to_timeval(const std::chrono::microseconds& c)
{
	std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(c);

	timeval tv;
	tv.tv_sec  = sec.count();
	tv.tv_usec = (c - sec).count();

	return tv;
}
std::chrono::microseconds timeval_to_chrono(const timeval& tv)
{
	return std::chrono::seconds(tv.tv_sec) + std::chrono::microseconds(tv.tv_usec);
}
