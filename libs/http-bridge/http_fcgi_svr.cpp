/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "http-bridge/http_fcgi_svr.hpp"

#include <spdlog/spdlog.h>

http_fcgi_svr::http_fcgi_svr()
{
  m_sock_fd = -1;
}

http_fcgi_svr::~http_fcgi_svr()
{
  if( ! m_thread_pool.empty() )
  {
    stop();
  }
}

bool http_fcgi_svr::start(const char* bind_addr, const size_t num_threads)
{
  SPDLOG_INFO("Starting server");

  SPDLOG_INFO("Binding to {:s}", bind_addr);
  m_sock_fd = FCGX_OpenSocket(bind_addr, 1024);

  if(m_sock_fd < 0)
  {
    SPDLOG_ERROR("Server failed to open socket"); 
    return false;
  }
 
  SPDLOG_INFO("FCGX_Init");
  FCGX_Init();

  SPDLOG_INFO("Staring threads");
  m_thread_pool.resize(num_threads);
  for(size_t i = 0; i < m_thread_pool.size(); i++)
  {
    m_thread_pool[i] = std::make_shared<http_fcgi_work_thread>();
    if( ! m_thread_pool[i]->init(this, m_sock_fd) )
    {
      SPDLOG_INFO("Thread init failed");
      return false;
    }
    m_thread_pool[i]->launch();
  }

  SPDLOG_INFO("Server started");
  return true;
}

bool http_fcgi_svr::stop()
{
  bool ret = true;
  SPDLOG_INFO("Stopping server");

  //shutdown FCGI
  FCGX_ShutdownPending();

  //interrupt all threads
  for(size_t i = 0; i < m_thread_pool.size(); i++)
  {
    m_thread_pool[i]->interrupt();
  }

  //close socket
  if(m_sock_fd != -1)
  {
    int ret = shutdown(m_sock_fd, SHUT_RDWR);
    if(ret != 0)
    {
      SPDLOG_ERROR("Error shutting down socket: {:d}", errno);
      ret = false;
    }

    ret = close(m_sock_fd);
    if(ret != 0)
    {
      SPDLOG_ERROR("Error closing socket: {:d}", errno);
      ret = false;
    }
    m_sock_fd = -1;
  }

  //join all threads
  for(size_t i = 0; i < m_thread_pool.size(); i++)
  {
    m_thread_pool[i]->join();
    m_thread_pool[i].reset();
  }
  m_thread_pool.clear();

  SPDLOG_INFO("Server stopped");

  return ret;
}

void http_fcgi_svr::register_cb_for_doc_uri(const char* doc_uri, const std::shared_ptr<http_req_callback_base>& cb)
{
  m_cb_table.set_node(doc_uri, std::make_shared<http_fcgi_svr_cb>(cb));
}
std::shared_ptr<http_req_callback_base> http_fcgi_svr::get_cb_for_doc_uri(const char* doc_uri)
{
  Directory_tree_node::ptr node = m_cb_table.find_match(doc_uri, Directory_tree::MATCH_TYPE::EXACT);
  if( ! node )
  {
    node = m_cb_table.find_match(doc_uri, Directory_tree::MATCH_TYPE::PARENT_PATH);
  }

  std::shared_ptr<http_req_callback_base> cb;
  if(node)
  {
    auto cb_ptr = node->get_data<http_fcgi_svr_cb>();
    if(cb_ptr)
    {
      cb = cb_ptr->m_cb;
    }
  }

  return cb;
}
