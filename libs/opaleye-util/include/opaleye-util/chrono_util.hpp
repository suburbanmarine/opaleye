/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2022 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <sys/time.h>

#include <chrono>

template <typename Rep, typename Period>
timeval chrono_to_timeval(const std::chrono::duration<Rep, Period>& c)
{
	return chrono_to_timeval(std::chrono::ceil<std::chrono::microseconds>(c));
}

timeval chrono_to_timeval(const std::chrono::microseconds& c);
std::chrono::microseconds timeval_to_chrono(const timeval& tv);

template <typename Rep, typename Period>
timespec chrono_to_timespec(const std::chrono::duration<Rep, Period>& c)
{
    return chrono_to_timeval(std::chrono::ceil<std::chrono::nanoseconds>(c));
}

timespec chrono_to_timespec(const std::chrono::nanoseconds& c);
std::chrono::nanoseconds timespec_to_chrono(const timespec& tv);

template <typename Rep, typename Period>
timespec timespec_add_chrono(const timespec& ts, const std::chrono::duration<Rep, Period>& c)
{
    return timespec_add_chrono(ts, std::chrono::ceil<std::chrono::nanoseconds>(c));
}

timespec timespec_add_chrono(const timespec& ts, const std::chrono::nanoseconds& c);