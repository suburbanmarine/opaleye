#include "http_req_jsonrpc.hpp"
#include "http_util.hpp"

#include <spdlog/spdlog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

void http_req_jsonrpc::handle(FCGX_Request* const request)
{
  if(true)
  {
    FCGX_PutS("Cache-Control: max-age=0, no-store\r\n", request->out);
    
    FCGX_PutS("\r\n", request->out);
  }
  else
  {
    FCGX_PutS("Status: 500 Internal Error\r\n", request->out);
  }

  FCGX_Finish_r(request);
}
