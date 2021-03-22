#include "http_req_jsonrpc.hpp"
#include "http_util.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

void http_req_jsonrpc::handle(FCGX_Request* const request)
{
  const int MAX_REQ_LEN = 16*1024;

  FCGX_PutS("Cache-Control: max-age=0, no-store\r\n", request->out);
  FCGX_PutS("\r\n", request->out);
  
  SPDLOG_INFO("GATEWAY_INTERFACE: {:s}", FCGX_GetParam("GATEWAY_INTERFACE", request->envp));
  SPDLOG_INFO("SERVER_SOFTWARE: {:s}",   FCGX_GetParam("SERVER_SOFTWARE", request->envp));
  SPDLOG_INFO("QUERY_STRING: {:s}",      FCGX_GetParam("QUERY_STRING", request->envp));
  SPDLOG_INFO("REQUEST_METHOD: {:s}",    FCGX_GetParam("REQUEST_METHOD", request->envp));
  SPDLOG_INFO("CONTENT_TYPE: {:s}",      FCGX_GetParam("CONTENT_TYPE", request->envp));
  SPDLOG_INFO("CONTENT_LENGTH: {:s}",    FCGX_GetParam("CONTENT_LENGTH", request->envp));
  SPDLOG_INFO("SCRIPT_FILENAME: {:s}",   FCGX_GetParam("SCRIPT_FILENAME", request->envp));
  SPDLOG_INFO("SCRIPT_NAME: {:s}",       FCGX_GetParam("SCRIPT_NAME", request->envp));
  SPDLOG_INFO("REQUEST_URI: {:s}",       FCGX_GetParam("REQUEST_URI", request->envp));
  SPDLOG_INFO("DOCUMENT_URI: {:s}",      FCGX_GetParam("DOCUMENT_URI", request->envp));
  SPDLOG_INFO("DOCUMENT_ROOT: {:s}",     FCGX_GetParam("DOCUMENT_ROOT", request->envp));
  SPDLOG_INFO("SERVER_PROTOCOL: {:s}",   FCGX_GetParam("SERVER_PROTOCOL", request->envp));
  SPDLOG_INFO("REMOTE_ADDR: {:s}",       FCGX_GetParam("REMOTE_ADDR", request->envp));
  SPDLOG_INFO("REMOTE_PORT: {:s}",       FCGX_GetParam("REMOTE_PORT", request->envp));
  SPDLOG_INFO("SERVER_ADDR: {:s}",       FCGX_GetParam("SERVER_ADDR", request->envp));
  SPDLOG_INFO("SERVER_PORT: {:s}",       FCGX_GetParam("SERVER_PORT", request->envp));
  SPDLOG_INFO("SERVER_NAME: {:s}",       FCGX_GetParam("SERVER_NAME", request->envp));

  //DOCUMENT_URI is just path
  char const * const DOCUMENT_URI = FCGX_GetParam("DOCUMENT_URI", request->envp);
  //REQUEST_URI is path and query string
  char const * const REQUEST_URI = FCGX_GetParam("REQUEST_URI", request->envp);
  //verify this is application/json; charset=UTF-8
  char const * const CONTENT_TYPE = FCGX_GetParam("CONTENT_TYPE", request->envp);
  //verify content size is sane
  char const * const CONTENT_LENGTH = FCGX_GetParam("CONTENT_LENGTH", request->envp);

  int req_len = 0;
  int ret = sscanf(CONTENT_LENGTH, "%d", &req_len);
  if(ret != 1)
  {
    throw std::invalid_argument("Could not parse CONTENT_LENGTH");
  }

  if((req_len < 0) || (req_len > MAX_REQ_LEN))
  {
    throw std::invalid_argument("CONTENT_LENGTH is invalid");
  }

  std::vector<char> m_req_buf;
  m_req_buf.resize(req_len);

  const int bytes_in = FCGX_GetStr(m_req_buf.data(), m_req_buf.size(), request->in);
  if(bytes_in != req_len)
  {
    throw std::runtime_error("Read did not return expected length");
  }

  SPDLOG_INFO("PAYLOAD: {:.{}}", m_req_buf.data(), m_req_buf.size());
  
  FCGX_PutS("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32000, \"message\": \"Server Error\"}, \"id\": 1}", request->out);

  FCGX_Finish_r(request);
}
