/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "http-bridge/http_req_callback_file.hpp"
#include "http-bridge/http_util.hpp"

#include <spdlog/spdlog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

void http_req_callback_file::handle(FCGX_Request* const request)
{
  const char fname[] = "/home/rounin/image-18.jpg";

  int fd = open(fname, O_RDONLY);
  if(fd >= 0)
  {
    struct stat statbuf;
    int ret = fstat(fd, &statbuf);
    if(ret == 0)
    {
      http_util::HttpDateStr date_str;
      if(http_util::timespec_to_httpdate(statbuf.st_mtim, &date_str))
      {
        FCGX_PutS("Content-Type: image/jpeg\r\n", request->out);
        FCGX_FPrintF(request->out, "Content-Length: %d\r\n", statbuf.st_size);
        FCGX_FPrintF(request->out, "Last-Modified: %s\r\n",  date_str.data());
        FCGX_PutS("Cache-Control: max-age=0, no-store\r\n", request->out);
        // FCGX_PutS("X-Accel-Buffering: yes\r\n", request->out);
        
        FCGX_PutS("\r\n", request->out);

        bool keep_reading = true;
        ssize_t read_ret;
        do
        {
          read_ret = read(fd, m_buf.data(), m_buf.size());
          if(read_ret < 0)
          {
            switch(errno)
            {
              case EINTR:
              {
                //continue
                break;
              }
              default:
              {
                //TODO: log errno
                SPDLOG_ERROR("Error reading from file: {:d}", errno);
                //stop
                keep_reading = false;
                break;
              }
            }
          }
          else if(read_ret == 0)
          {
            keep_reading = false;
          }
          else
          {
            FCGX_PutStr(m_buf.data(), read_ret, request->out);
          }
        } while(keep_reading);

        // FCGX_SetExitStatus(0, request->out);

        close(fd);
      }
      else
      {
        FCGX_PutS("Status: 500 Internal Error\r\n", request->out);
      }
    }
    else
    {
      FCGX_PutS("Status: 500 Internal Error\r\n", request->out);
    }
  }
  else
  {
    FCGX_PutS("Status: 500 Internal Error\r\n", request->out);
  }

  FCGX_Finish_r(request);
}
