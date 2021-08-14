/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <thread>

#include "http_fcgi_svr_fwd.hpp"

#include <atomic>

class http_fcgi_work_thread
{
public:

  http_fcgi_work_thread();
  ~http_fcgi_work_thread();

  void launch(http_fcgi_svr* svr, int sock_fd);

  void work();

  void interrupt();

  //MT safe
  void join();

protected:
  std::atomic<bool> m_keep_running;
  http_fcgi_svr* m_svr;
  int m_sock_fd;
  std::thread m_thread;
};
