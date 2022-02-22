/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "http_req_jpeg.hpp"

#include "http-bridge/http_req_util.hpp"
#include "http-bridge/http_util.hpp"
#include "http-bridge/http_common.hpp"

#include <boost/filesystem/path.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <spdlog/spdlog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

void http_req_jpeg::handle(FCGX_Request* const request)
{
  http_req_util req_util;
  req_util.load(request);
  req_util.log_request_env();

  if(req_util.request_method_enum != http_common::REQUEST_METHOD::GET)
  {
    throw BadRequest("Only GET is accepted");
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
