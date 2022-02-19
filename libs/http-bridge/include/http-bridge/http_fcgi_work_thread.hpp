/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "http-bridge/http_fcgi_svr_fwd.hpp"

#include "thread_base.hpp"

class http_fcgi_work_thread : public thread_base
{
public:

  http_fcgi_work_thread();
  ~http_fcgi_work_thread();

  bool init(http_fcgi_svr* svr, int sock_fd);

  void work() override;

protected:
  http_fcgi_svr* m_svr;
  int m_sock_fd;
};
