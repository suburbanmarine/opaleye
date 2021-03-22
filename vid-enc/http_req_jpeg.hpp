#pragma once

#include "http_req_callback_base.hpp"

#include "pipeline/camera/Logitech_brio_pipe.hpp"

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

class http_req_jpeg : public http_req_callback_base
{
public:

  http_req_jpeg()
  {
    m_cam = nullptr;
  }

  ~http_req_jpeg() override
  {
    
  }

  void handle(FCGX_Request* const request) override;

  void set_cam(Logitech_brio_pipe* cam)
  {
    m_cam = cam;
  }

protected:

  Logitech_brio_pipe* m_cam;

};
