/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "http_req_jpeg.hpp"
#include "http_util.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <spdlog/spdlog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

void http_req_jpeg::handle(FCGX_Request* const request)
{
  //TODO move this to base class
  if(true)
  { 
    boost::property_tree::ptree temp;

    temp.put<std::string>("QUERY_STRING",    FCGX_GetParam("QUERY_STRING",    request->envp));
    temp.put<std::string>("REQUEST_METHOD",  FCGX_GetParam("REQUEST_METHOD",  request->envp));
    temp.put<std::string>("CONTENT_TYPE",    FCGX_GetParam("CONTENT_TYPE",    request->envp));
    temp.put<std::string>("CONTENT_LENGTH",  FCGX_GetParam("CONTENT_LENGTH",  request->envp));
    temp.put<std::string>("SCRIPT_NAME",     FCGX_GetParam("SCRIPT_NAME",     request->envp));
    temp.put<std::string>("REQUEST_URI",     FCGX_GetParam("REQUEST_URI",     request->envp));
    temp.put<std::string>("DOCUMENT_URI",    FCGX_GetParam("DOCUMENT_URI",    request->envp));
    temp.put<std::string>("DOCUMENT_ROOT",   FCGX_GetParam("DOCUMENT_ROOT",   request->envp));
    temp.put<std::string>("SERVER_PROTOCOL", FCGX_GetParam("SERVER_PROTOCOL", request->envp));
    temp.put<std::string>("REQUEST_SCHEME",  FCGX_GetParam("REQUEST_SCHEME",  request->envp));
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

  //this is per-req since we could have several threads
  std::vector<uint8_t> frame_buf;
  m_cb(&frame_buf);
  
  if( ! frame_buf.empty() )
  {
    time_t t_now = time(NULL);
    http_util::HttpDateStr time_str;
    if( ! http_util::time_to_httpdate(t_now, &time_str) )
    {
      throw InternalServerError("Could not get Last-Modified timestamp");
    }

    FCGX_PutS("Content-Type: image/jpeg\r\n", request->out);
    FCGX_FPrintF(request->out, "Content-Length: %d\r\n", frame_buf.size());
    FCGX_PutS("Cache-Control: max-age=0, no-store\r\n", request->out);
    FCGX_FPrintF(request->out, "Last-Modified: %s\r\n", time_str.data());
    // FCGX_PutS("Cache-Control: max-age=2, public\r\n", request->out);
    // FCGX_PutS("X-Accel-Buffering: yes\r\n", request->out);
    
    FCGX_PutS("\r\n", request->out);

    FCGX_PutStr(reinterpret_cast<const char*>(frame_buf.data()), frame_buf.size(), request->out);
  }
  else
  {
    FCGX_PutS("Status: 500 Internal Error\r\n", request->out);
  }

  FCGX_Finish_r(request);
}
