/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "http-bridge/http_req_callback_base.hpp"

#include "http-bridge/http_common.hpp"

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

#include <array>

class http_req_error_handler : public http_req_callback_base
{
public:

  http_req_error_handler()
  {
    m_status_code = http_common::STATUS_CODES::OK;
  }

  ~http_req_error_handler() override
  {
    
  }

  void handle(FCGX_Request* const request) override;

protected:

http_common::STATUS_CODES m_status_code;

};
