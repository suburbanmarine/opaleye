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
  { 
    SPDLOG_INFO("QUERY_STRING: {:s}",      FCGX_GetParam("QUERY_STRING", request->envp));
    SPDLOG_INFO("REQUEST_METHOD: {:s}",    FCGX_GetParam("REQUEST_METHOD", request->envp));
    SPDLOG_INFO("CONTENT_TYPE: {:s}",      FCGX_GetParam("CONTENT_TYPE", request->envp));
    SPDLOG_INFO("CONTENT_LENGTH: {:s}",    FCGX_GetParam("CONTENT_LENGTH", request->envp));

    SPDLOG_INFO("SCRIPT_NAME: {:s}",       FCGX_GetParam("SCRIPT_NAME", request->envp));
    SPDLOG_INFO("REQUEST_URI: {:s}",       FCGX_GetParam("REQUEST_URI", request->envp));
    SPDLOG_INFO("DOCUMENT_URI: {:s}",      FCGX_GetParam("DOCUMENT_URI", request->envp));
    SPDLOG_INFO("DOCUMENT_ROOT: {:s}",     FCGX_GetParam("DOCUMENT_ROOT", request->envp));
    SPDLOG_INFO("SERVER_PROTOCOL: {:s}",   FCGX_GetParam("SERVER_PROTOCOL", request->envp));
    SPDLOG_INFO("REQUEST_SCHEME: {:s}",    FCGX_GetParam("REQUEST_SCHEME", request->envp));
    char const * const HTTPS = FCGX_GetParam("HTTPS", request->envp);
    SPDLOG_INFO("HTTPS: {:s}", (HTTPS) ? (HTTPS) : ("<null>"));

    SPDLOG_INFO("GATEWAY_INTERFACE: {:s}", FCGX_GetParam("GATEWAY_INTERFACE", request->envp));
    SPDLOG_INFO("SERVER_SOFTWARE: {:s}",   FCGX_GetParam("SERVER_SOFTWARE", request->envp));

    SPDLOG_INFO("REMOTE_ADDR: {:s}",       FCGX_GetParam("REMOTE_ADDR", request->envp));
    SPDLOG_INFO("REMOTE_PORT: {:s}",       FCGX_GetParam("REMOTE_PORT", request->envp));
    SPDLOG_INFO("SERVER_ADDR: {:s}",       FCGX_GetParam("SERVER_ADDR", request->envp));
    SPDLOG_INFO("SERVER_PORT: {:s}",       FCGX_GetParam("SERVER_PORT", request->envp));
    SPDLOG_INFO("SERVER_NAME: {:s}",       FCGX_GetParam("SERVER_NAME", request->envp));

    SPDLOG_INFO("REDIRECT_STATUS: {:s}",   FCGX_GetParam("REDIRECT_STATUS", request->envp));
  }

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
    throw BadRequest("Could not parse CONTENT_LENGTH");
  }

  if((req_len < 0) || (req_len > MAX_REQ_LEN))
  {
    throw BadRequest("CONTENT_LENGTH is invalid");
  }

  //validate CONTENT_TYPE, ignoring any optional charset
  { 
    const char app_jsonrpc[] = "application/json";
    if(strlen(CONTENT_TYPE) == 0)
    {
      throw BadRequest("CONTENT_LENGTH is invalid");
    }

    if(strncmp(CONTENT_TYPE, app_jsonrpc, sizeof(app_jsonrpc)-1) != 0)
    {
      throw BadRequest("CONTENT_LENGTH is invalid");
    }
  }

  //hold the jsonrpc response object
  std::shared_ptr<jsonrpc::FormattedData> result;

  //this part of the rpc is single threaded
  {
    std::lock_guard<std::mutex> lock(m_jsonrpc_mutex);

    //read to the vector, then the string
    {
      m_req_buf.resize(req_len);
      const int bytes_in = FCGX_GetStr(m_req_buf.data(), m_req_buf.size(), request->in);
      if(bytes_in != req_len)
      {
        throw InternalServerError("Read did not return expected length");
      }

      //copy to a string to make jsonrpc-lean happy
      //maybe patch jsonrpc-lean to take a string-view
      m_req_str.assign(m_req_buf.begin(), m_req_buf.end());

      SPDLOG_INFO("Request: {:.{}}",  m_req_buf.data(), m_req_buf.size());
    }


    // it wants CONTENT_TYPE == application/json - the application/json; charset=UTF-8 causes it to fail
    // std::shared_ptr<jsonrpc::FormattedData> result = m_jsonrpc_server_ptr->HandleRequest(m_req_str, CONTENT_TYPE);
    try
    {
      result = m_jsonrpc_server_ptr->HandleRequest(m_req_str);
    }
    catch(std::exception& e)
    {
      SPDLOG_ERROR("Error handling jsonrpc request: {:s}", e.what());
      throw InternalServerError("Error handling jsonrpc request");
    }
  }

  if(result)
  {
    SPDLOG_INFO("Response: {:.{}}", result->GetData(), result->GetSize());
  }
  else
  {
    SPDLOG_INFO("Response: NULL");
  }

  // -32700  Parse error Invalid JSON was received by the server.
  // An error occurred on the server while parsing the JSON text.
  // -32600  Invalid Request The JSON sent is not a valid Request object.
  // -32601  Method not found  The method does not exist / is not available.
  // -32602  Invalid params  Invalid method parameter(s).
  // -32603  Internal error  Internal JSON-RPC error.
  // -32000 to -32099  Server error  Reserved for implementation-defined server-errors.
  // FCGX_PutS("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32000, \"message\": \"Server Error\"}, \"id\": 1}", request->out);
  if(result)
  {
    //print response header
    FCGX_PutS("Cache-Control: max-age=0, no-store\r\n", request->out);
    FCGX_PutS("Content-type: application/json\r\n", request->out);
    FCGX_FPrintF(request->out, "Content-length: %d\r\n", result->GetSize());
    FCGX_PutS("\r\n", request->out);

    //print response body
    FCGX_PutStr(result->GetData(), result->GetSize(), request->out);
  }
  else
  {
    // JSON-RPC 2.0 says:
    // If there was an error in detecting the id in the Request object (e.g. Parse error/Invalid Request), it MUST be Null.
    // We could try harder to parse the request id here, but for now we treat it as a invalid request
    FCGX_PutS("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": -32000, \"message\": \"Server Error\"}, \"id\": null}", request->out);
  }
  FCGX_Finish_r(request);
}
