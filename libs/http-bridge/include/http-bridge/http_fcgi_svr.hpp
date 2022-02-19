/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "http-bridge/http_req_callback_base.hpp"
#include "http-bridge/http_fcgi_work_thread.hpp"

#include "opaleye-util/path/Directory_tree.hpp"

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <memory>
#include <vector>

class http_fcgi_svr_cb : public Directory_tree_node::Data
{
public:
  
  http_fcgi_svr_cb(const std::shared_ptr<http_req_callback_base>& cb) : m_cb(cb)
  {

  }

  std::shared_ptr<http_req_callback_base> m_cb;
};

class http_fcgi_svr
{
public:

  http_fcgi_svr();

  ~http_fcgi_svr();

  bool start(const char* bind_addr, const size_t num_threads);

  bool stop();

  void register_cb_for_doc_uri(const char* doc_uri, const std::shared_ptr<http_req_callback_base>& cb);
  std::shared_ptr<http_req_callback_base> get_cb_for_doc_uri(const char* doc_uri);

protected:
  int m_sock_fd;

  std::vector<std::shared_ptr<http_fcgi_work_thread>> m_thread_pool;

  // exact match
  // map doc uri to callback handler
  // std::map<std::string, std::shared_ptr<http_req_callback_base>> m_cb_table;
  Directory_tree m_cb_table;

  // parent dir match
  // TODO: map dir prefix to cb
  // std::map<std::string, std::shared_ptr<http_req_callback_base>> m_cb_parent_table;

  // regex match
  // TODO: map regex to cb 
  // std::list<std::shared_ptr<boost::regex>, std::shared_ptr<http_req_callback_base>> m_cb_regex_table;
};
