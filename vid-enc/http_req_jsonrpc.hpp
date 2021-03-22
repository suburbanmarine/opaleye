#pragma once

#include "http_req_callback_base.hpp"

#include "pipeline/camera/Logitech_brio_pipe.hpp"

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

class http_req_jsonrpc : public http_req_callback_base
{
public:

  http_req_jsonrpc()
  {
    
  }

  ~http_req_jsonrpc() override
  {
    
  }

  void handle(FCGX_Request* const request) override;

protected:

};
