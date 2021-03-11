#pragma once

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

class http_req_callback_base
{
public:

  http_req_callback_base()
  {

  }

  virtual ~http_req_callback_base()
  {
    
  }

  virtual void handle(FCGX_Request* const req)
  {
    
  }

protected:

};
