#include "http_req_error_handler.hpp"

void http_req_error_handler::handle(FCGX_Request* const request)
{
  const char* code_msg = http_common::get_status_code_str(m_status_code);

  FCGX_PutS("Content-Type: text/html\r\n", request->out);
  FCGX_FPrintF(request->out, "Status: {:d} {:s}\r\n", 
      m_status_code,
      code_msg
    );
  FCGX_PutS("\r\n", request->out);
  FCGX_PutS(code_msg, request->out);

  FCGX_Finish_r(request);
}
