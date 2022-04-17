/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "http-bridge/http_util.hpp"

bool http_util::time_to_httpdate(const time_t& t, HttpDateStr* out_str)
{
  struct tm tm_buf;
  if(gmtime_r(&t, &tm_buf) != &tm_buf)
  {
    return false;
  }

  const int ret = snprintf(out_str->data(), out_str->size(), "%s, %d %s %d %02d:%02d:%02d GMT", 
      DAY_STR[tm_buf.tm_wday],
      tm_buf.tm_mday,
      MONTH_STR[tm_buf.tm_mon],
      tm_buf.tm_year + 1900,
      tm_buf.tm_hour,
      tm_buf.tm_min,
      tm_buf.tm_sec
    );

  return (ret > 0) && (size_t(ret) < out_str->size());  
}

bool http_util::timespec_to_httpdate(const timespec& ts, HttpDateStr* out_str)
{
  // struct tm tm_buf;
  // if(gmtime_r(&ts.tv_sec, &tm_buf) != &tm_buf)
  // {
  //   return false;
  // }

  // const int ret = snprintf(out_str->data(), out_str->size(), "%s, %d %s %d %02d:%02d:%02d.%09ld GMT", 
  //     DAY_STR[tm_buf.tm_wday],
  //     tm_buf.tm_mday,
  //     MONTH_STR[tm_buf.tm_mon],
  //     tm_buf.tm_year + 1900,
  //     tm_buf.tm_hour,
  //     tm_buf.tm_min,
  //     tm_buf.tm_sec,
  //     ts.tv_nsec
  //   );

  // return (ret > 0) && (size_t(ret) < out_str->size());
  return time_to_httpdate(ts.tv_sec, out_str);
}
bool http_util::timeval_to_httpdate(const timeval& tv, HttpDateStr* out_str)
{
  // struct tm tm_buf;
  // if(gmtime_r(&ts.tv_sec, &tm_buf) != &tm_buf)
  // {
  //   return false;
  // }

  // const int ret = snprintf(out_str->data(), out_str->size(), "%s, %d %s %d %02d:%02d:%02d.%06ld GMT", 
  //     DAY_STR[tm_buf.tm_wday],
  //     tm_buf.tm_mday,
  //     MONTH_STR[tm_buf.tm_mon],
  //     tm_buf.tm_year + 1900,
  //     tm_buf.tm_hour,
  //     tm_buf.tm_min,
  //     tm_buf.tm_sec,
  //     ts.tv_usec
  //   );

  // return (ret > 0) && (size_t(ret) < out_str->size());
  return time_to_httpdate(tv.tv_sec, out_str);
}
char const * const http_util::DAY_STR[7]    = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char const * const http_util::MONTH_STR[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
