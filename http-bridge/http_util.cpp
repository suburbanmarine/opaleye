#include "http_util.hpp"

bool http_util::timespec_to_httpdate(const timespec& ts, HttpDateStr* out_str)
{
  struct tm tm_buf;
  if(gmtime_r(&ts.tv_sec, &tm_buf) != &tm_buf)
  {
    return false;
  }

  const int ret = snprintf(out_str->data(), out_str->size(), "%s, %d %s %d %02d:%02d:%02d.%09ld GMT", 
      DAY_STR[tm_buf.tm_wday],
      tm_buf.tm_mday,
      MONTH_STR[tm_buf.tm_mon],
      tm_buf.tm_year + 1900,
      tm_buf.tm_hour,
      tm_buf.tm_min,
      tm_buf.tm_sec,
      ts.tv_nsec
    );

  return (ret > 0) && (size_t(ret) < out_str->size());
}

char const * const http_util::DAY_STR[7]    = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char const * const http_util::MONTH_STR[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
