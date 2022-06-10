/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "http-bridge/http_fcgi_work_thread.hpp"

#include "http-bridge/http_fcgi_svr.hpp"

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <cstring>

http_fcgi_work_thread::http_fcgi_work_thread()
{
  m_svr     = nullptr;
  m_sock_fd = -1;
}

http_fcgi_work_thread::~http_fcgi_work_thread()
{
  if(m_thread.joinable())
  {
    //signal thread?!? sigalarm?
    //this may block, may need to send signal to thread
    interrupt();
    join();
  }
}

bool http_fcgi_work_thread::init(http_fcgi_svr* svr, int sock_fd)
{
  m_svr = svr;
  m_sock_fd = sock_fd;

  return true;
}

void http_fcgi_work_thread::work()
{
  SPDLOG_INFO("thread starting: {}", std::this_thread::get_id());

  FCGX_Request request;
  memset(&request, 0, sizeof(FCGX_Request));

  FCGX_InitRequest(&request, m_sock_fd, FCGI_FAIL_ACCEPT_ON_INTR);

  while( ! is_interrupted() )
  {
    if(FCGX_Accept_r(&request) == 0)
    {
      //check if running
      if( is_interrupted() )
      {
        SPDLOG_INFO("interruption requested: {}", std::this_thread::get_id());

        const char msg[]     = "Internal Error";
        const char msg_len = sizeof(msg) - 1;
        FCGX_PutS("Content-Type: text/html\r\n", request.out);
        FCGX_FPrintF(request.out, "Content-Length: %d\r\n", msg_len);
        FCGX_PutS("Status: 500 Internal Error\r\n", request.out);
        FCGX_PutS("\r\n", request.out);
        FCGX_FPrintF(request.out, "%s", msg);
        FCGX_Finish_r(&request);

        break;
      }

      // std::cout << "GATEWAY_INTERFACE" << ": " << FCGX_GetParam("GATEWAY_INTERFACE", request.envp) << std::endl;
      // std::cout << "SERVER_SOFTWARE" << ": " << FCGX_GetParam("SERVER_SOFTWARE", request.envp) << std::endl;
      // std::cout << "QUERY_STRING" << ": " << FCGX_GetParam("QUERY_STRING", request.envp) << std::endl;
      // std::cout << "REQUEST_METHOD" << ": " << FCGX_GetParam("REQUEST_METHOD", request.envp) << std::endl;
      // std::cout << "CONTENT_TYPE" << ": " << FCGX_GetParam("CONTENT_TYPE", request.envp) << std::endl;
      // std::cout << "CONTENT_LENGTH" << ": " << FCGX_GetParam("CONTENT_LENGTH", request.envp) << std::endl;
      // std::cout << "SCRIPT_FILENAME" << ": " << FCGX_GetParam("SCRIPT_FILENAME", request.envp) << std::endl;
      // std::cout << "SCRIPT_NAME" << ": " << FCGX_GetParam("SCRIPT_NAME", request.envp) << std::endl;
      // std::cout << "REQUEST_URI" << ": " << FCGX_GetParam("REQUEST_URI", request.envp) << std::endl;
      // std::cout << "DOCUMENT_URI" << ": " << FCGX_GetParam("DOCUMENT_URI", request.envp) << std::endl;
      // std::cout << "DOCUMENT_ROOT" << ": " << FCGX_GetParam("DOCUMENT_ROOT", request.envp) << std::endl;
      // std::cout << "SERVER_PROTOCOL" << ": " << FCGX_GetParam("SERVER_PROTOCOL", request.envp) << std::endl;
      // std::cout << "REMOTE_ADDR" << ": " << FCGX_GetParam("REMOTE_ADDR", request.envp) << std::endl;
      // std::cout << "REMOTE_PORT" << ": " << FCGX_GetParam("REMOTE_PORT", request.envp) << std::endl;
      // std::cout << "SERVER_ADDR" << ": " << FCGX_GetParam("SERVER_ADDR", request.envp) << std::endl;
      // std::cout << "SERVER_PORT" << ": " << FCGX_GetParam("SERVER_PORT", request.envp) << std::endl;
      // std::cout << "SERVER_NAME" << ": " << FCGX_GetParam("SERVER_NAME", request.envp) << std::endl;
        
      // eg only handle get
      const char* REQUEST_METHOD = FCGX_GetParam("REQUEST_METHOD", request.envp);
      
      // this includes params
      const char* REQUEST_URI = FCGX_GetParam("REQUEST_URI", request.envp);

      // this is just the path to doc
      const char* DOCUMENT_URI = FCGX_GetParam("DOCUMENT_URI", request.envp);

      // this is servers configured doc root for this uri
      const char* DOCUMENT_ROOT = FCGX_GetParam("DOCUMENT_ROOT", request.envp);

      SPDLOG_DEBUG("REQUEST_METHOD: {:s}", REQUEST_METHOD);
      SPDLOG_DEBUG("REQUEST_URI: {:s}", REQUEST_URI);
      SPDLOG_DEBUG("DOCUMENT_URI: {:s}", DOCUMENT_URI);
      SPDLOG_DEBUG("DOCUMENT_ROOT: {:s}", DOCUMENT_ROOT);

      //lookup request handler by uri
      std::shared_ptr<http_req_callback_base> req_cb;
      {
        req_cb = m_svr->get_cb_for_doc_uri(DOCUMENT_URI);
      }

      //handle
      if(req_cb)
      {
        try
        {
          req_cb->handle(&request);
        }
        catch(const HTTPException& e)
        {
          SPDLOG_ERROR("Caught exception: {:s}", e.what());
          FCGX_PutS("Content-Type: text/html\r\n", request.out);
          FCGX_FPrintF(request.out, "Content-Length: %d\r\n", strlen(e.what()));
          FCGX_FPrintF(request.out, "Status: %d %s\r\n", e.get_code(), e.what());
          FCGX_PutS("\r\n", request.out);
          FCGX_FPrintF(request.out, "%s\r\n", e.what());
          FCGX_Finish_r(&request);
        }
        catch(const std::exception& e)
        {
          const char msg[]     = "Internal Error";
          const char msg_len = sizeof(msg) - 1;
          SPDLOG_ERROR("Caught exception: {:s}", e.what());
          FCGX_PutS("Content-Type: text/html\r\n", request.out);
          FCGX_FPrintF(request.out, "Content-Length: %d\r\n", msg_len);
          FCGX_PutS("Status: 500 Internal Error\r\n", request.out);
          FCGX_PutS("\r\n", request.out);
          FCGX_FPrintF(request.out, "%s", msg);
          FCGX_Finish_r(&request);
        }
      }
      else
      {
        const char msg[]     = "Not Found";
        const char msg_len = sizeof(msg) - 1;

        FCGX_PutS("Content-Type: text/html\r\n", request.out);
        FCGX_FPrintF(request.out, "Content-Length: %d\r\n", msg_len);
        FCGX_PutS("Status: 404 Not Found\r\n", request.out);
        FCGX_PutS("\r\n", request.out);
        FCGX_FPrintF(request.out, "%s", msg);
        FCGX_Finish_r(&request);
      }
    }
  }

  FCGX_Free(&request, m_sock_fd);

  SPDLOG_INFO("thread stoppping: {}", std::this_thread::get_id());
}
