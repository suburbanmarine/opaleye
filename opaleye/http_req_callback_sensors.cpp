/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "http_req_callback_sensors.hpp"
#include "http_util.hpp"

#include "linux_thermal_zone.hpp"

#include <Unit_conv.hpp>

#include "http_common.hpp"

#include <boost/filesystem/path.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include <sstream>

void http_req_callback_sensors::handle(FCGX_Request* const request)
{
  //TODO move this to base class
  if(true)
  { 
    boost::property_tree::ptree temp;

    temp.put<std::string>("QUERY_STRING",      FCGX_GetParam("QUERY_STRING",    request->envp));
    temp.put<std::string>("REQUEST_METHOD",    FCGX_GetParam("REQUEST_METHOD",  request->envp));
    temp.put<std::string>("CONTENT_TYPE",      FCGX_GetParam("CONTENT_TYPE",    request->envp));
    temp.put<std::string>("CONTENT_LENGTH",    FCGX_GetParam("CONTENT_LENGTH",  request->envp));
    temp.put<std::string>("SCRIPT_NAME",       FCGX_GetParam("SCRIPT_NAME",     request->envp));
    temp.put<std::string>("REQUEST_URI",       FCGX_GetParam("REQUEST_URI",     request->envp));
    temp.put<std::string>("DOCUMENT_URI",      FCGX_GetParam("DOCUMENT_URI",    request->envp));
    temp.put<std::string>("DOCUMENT_ROOT",     FCGX_GetParam("DOCUMENT_ROOT",   request->envp));
    temp.put<std::string>("SERVER_PROTOCOL",   FCGX_GetParam("SERVER_PROTOCOL", request->envp));
    temp.put<std::string>("REQUEST_SCHEME",    FCGX_GetParam("REQUEST_SCHEME",  request->envp));
    char const * const HTTPS = FCGX_GetParam("HTTPS", request->envp);
    temp.put<std::string>("HTTPS", (HTTPS) ? (HTTPS) : ("<null>"));
    temp.put<std::string>("GATEWAY_INTERFACE", FCGX_GetParam("GATEWAY_INTERFACE", request->envp));
    temp.put<std::string>("SERVER_SOFTWARE",   FCGX_GetParam("SERVER_SOFTWARE",   request->envp));
    temp.put<std::string>("REMOTE_ADDR",       FCGX_GetParam("REMOTE_ADDR",       request->envp));
    temp.put<std::string>("REMOTE_PORT",       FCGX_GetParam("REMOTE_PORT",       request->envp));
    temp.put<std::string>("SERVER_ADDR",       FCGX_GetParam("SERVER_ADDR",       request->envp));
    temp.put<std::string>("SERVER_PORT",       FCGX_GetParam("SERVER_PORT",       request->envp));
    temp.put<std::string>("SERVER_NAME",       FCGX_GetParam("SERVER_NAME",       request->envp));
    temp.put<std::string>("REDIRECT_STATUS",   FCGX_GetParam("REDIRECT_STATUS",   request->envp));

    std::stringstream ss;
    boost::property_tree::write_json(ss, temp);
    SPDLOG_DEBUG("request info: {:s}", ss.str());
  }

  boost::filesystem::path DOCUMENT_URI       = FCGX_GetParam("DOCUMENT_URI",    request->envp);

  {
    http_common::REQUEST_METHOD REQUEST_METHOD = http_common::parse_req_method(FCGX_GetParam("REQUEST_METHOD", request->envp));
    if(REQUEST_METHOD != http_common::REQUEST_METHOD::GET)
    {
      throw BadRequest("Only GET is accepted");
    }
  }

  //this is per-req since we could have several threads
  double ext_temp_data = 0.0;
  m_sensors->get_temp_data(&ext_temp_data);

  MS5837_30BA::RESULT baro_data = {};
  m_sensors->get_baro_data(&baro_data);

  std::map<std::string, double> soc_temp;
  linux_thermal_zone lz;
  if(lz.sample())
  {
    lz.get_temps(&soc_temp);
  }

  
  if( true ) // if have data
  {
    std::stringstream ss;

    ss << fmt::format("External\r\n");
    ss << fmt::format("\tTemp: {:0.1f} degC / {:0.1f} degF\r\n", ext_temp_data, Unit_conv::degC_to_degF(ext_temp_data));
    ss << fmt::format("\tPres: {:d} mbar\r\n", baro_data.P1_mbar);
    ss << fmt::format("\r\n");
    ss << fmt::format("Internal\r\n");
    for(const auto t : soc_temp)
    {
      ss << fmt::format("\t{:s}: {:0.3f} degC\r\n", t.first, t.second);
    }


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
