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
