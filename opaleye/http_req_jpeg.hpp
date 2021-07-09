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
    
  }

  ~http_req_jpeg() override
  {
    
  }

  void handle(FCGX_Request* const request) override;

  typedef std::function<void(std::vector<uint8_t>*)> GetImageCb;
  void set_get_image_cb(const GetImageCb& cb)
  {
    m_cb = cb;
  }

protected:

  GetImageCb m_cb;

  std::shared_ptr<std::vector<uint8_t>> m_buf;

};
