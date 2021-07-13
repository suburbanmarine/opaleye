#include "http_req_callback_sensors.hpp"
#include "http_util.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include <sstream>

void http_req_callback_sensors::handle(FCGX_Request* const request)
{
  //this is per-req since we could have several threads
  double ext_temp_data;
  m_sensors->get_temp_data(&ext_temp_data);

  MS5837_30BA::RESULT baro_data;
  m_sensors->get_baro_data(&baro_data);
  
  if( true ) // if have data
  {
    std::stringstream ss;

    ss << fmt::format("External Temp: {:0.3f} degC\r\n", ext_temp_data);
    ss << fmt::format("External Pres: {:d} mbar\r\n", baro_data.P1_mbar);

    std::string str = ss.str();

    time_t t_now = time(NULL);
    http_util::HttpDateStr time_str;
    if( ! http_util::time_to_httpdate(t_now, &time_str) )
    {
      throw InternalServerError("Could not get Last-Modified timestamp");
    }

    FCGX_PutS("Content-Type: text/plain\r\n", request->out);
    // FCGX_PutS("Content-Type: text/html\r\n", request->out);
    FCGX_FPrintF(request->out, "Content-Length: %d\r\n", str.size());
    FCGX_PutS("Cache-Control: max-age=0, no-store\r\n", request->out);
    FCGX_FPrintF(request->out, "Last-Modified: %s\r\n", time_str.data());
    
    FCGX_PutS("\r\n", request->out);

    FCGX_PutStr(str.data(), str.size(), request->out);
  }
  else
  {
    FCGX_PutS("Status: 500 Internal Error\r\n", request->out);
  }

  FCGX_Finish_r(request);
}
