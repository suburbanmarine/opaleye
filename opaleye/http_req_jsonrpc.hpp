/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "http_req_callback_base.hpp"

#include "pipeline/camera/Logitech_brio_pipe.hpp"

#include "jsonrpc-lean/server.h"

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

#include <memory>
#include <string>
#include <vector>
#include <mutex>

class http_req_jsonrpc : public http_req_callback_base
{
public:

  http_req_jsonrpc()
  {
    m_req_str.reserve(MAX_REQ_LEN);
    m_req_buf.reserve(MAX_REQ_LEN);
  }

  ~http_req_jsonrpc() override
  {
    
  }

  void handle(FCGX_Request* const request) override;

  void set_rpc_server(const std::shared_ptr<jsonrpc::Server>& server)
  {
    m_jsonrpc_server_ptr = server;
  }

protected:

  static constexpr int MAX_REQ_LEN = 16*1024;

  std::mutex m_jsonrpc_mutex;

  //idk if this is thread safe
  std::shared_ptr<jsonrpc::Server> m_jsonrpc_server_ptr;

  //these need to be thread local
  std::vector<char> m_req_buf;
  std::string m_req_str;
};
