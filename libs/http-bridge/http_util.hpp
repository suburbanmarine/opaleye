/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <ctime>
#include <cstdio>

#include <array>

class http_util
{
public:

  typedef std::array<char, 40> HttpDateStr;
  
  static bool time_to_httpdate(const time_t& t, HttpDateStr* out_str);
  static bool timespec_to_httpdate(const timespec& ts, HttpDateStr* out_str);
  static bool timeval_to_httpdate(const timeval& tv, HttpDateStr* out_str);

protected:
  static char const * const DAY_STR[7];
  static char const * const MONTH_STR[12];
};
