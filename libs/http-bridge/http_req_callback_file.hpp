#pragma once

#include "http_req_callback_base.hpp"

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

#include <array>

class http_req_callback_file : public http_req_callback_base
{
public:

  http_req_callback_file()
  {

  }

  ~http_req_callback_file() override
  {
    
  }

  void handle(FCGX_Request* const request) override;

protected:

  std::array<char, 4096> m_buf;

};
