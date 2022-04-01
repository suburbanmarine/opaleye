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

timespec chrono_to_timespec(const std::chrono::nanoseconds& c)
{
	std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(c);

	timespec ts;
	ts.tv_sec  = sec.count();
	ts.tv_nsec = (c - sec).count();

	return ts;
}
std::chrono::nanoseconds timespec_to_chrono(const timespec& tv)
{
	return std::chrono::seconds(tv.tv_sec) + std::chrono::nanoseconds(tv.tv_nsec);	
}

timespec timespec_add_chrono(const timespec& ts, const std::chrono::nanoseconds& c)
{
	//TODO verify wrap-around math on nsec
	
	std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(c);

	timespec out_ts;
	out_ts.tv_sec  = ts.tv_sec + sec.count();
	out_ts.tv_nsec = ts.tv_nsec + (c - sec).count();
	
	if(out_ts.tv_nsec > 999999999LL)
	{
		out_ts.tv_nsec -= 1000000000LL;
		out_ts.tv_sec++;
	}
	else if (out_ts.tv_nsec < 0)
	{
		out_ts.tv_nsec += 1000000000LL;
		out_ts.tv_sec--;
	}

	return out_ts;
}
