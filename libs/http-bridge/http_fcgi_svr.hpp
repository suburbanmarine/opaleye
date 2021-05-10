#pragma once

#include "http_req_callback_base.hpp"
#include "http_fcgi_work_thread.hpp"

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <memory>
#include <vector>

class http_fcgi_svr
{
public:

  http_fcgi_svr();

  ~http_fcgi_svr();

  bool start();

  bool stop();

  void register_cb_for_doc_uri(const char* doc_uri, const std::shared_ptr<http_req_callback_base>& cb);
  std::shared_ptr<http_req_callback_base> get_cb_for_doc_uri(const char* doc_uri);

protected:
  int m_sock_fd;

  std::vector<std::shared_ptr<http_fcgi_work_thread>> m_thread_pool;

  // map doc uri to callback handler
  std::map<std::string, std::shared_ptr<http_req_callback_base>> m_cb_table;
};
