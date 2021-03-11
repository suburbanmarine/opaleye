#pragma once

#include <ctime>
#include <cstdio>

#include <array>

class http_util
{
public:

  typedef std::array<char, 40> HttpDateStr;
  
  static bool timespec_to_httpdate(const timespec& ts, HttpDateStr* out_str);

protected:
  static char const * const DAY_STR[7];
  static char const * const MONTH_STR[12];
};
