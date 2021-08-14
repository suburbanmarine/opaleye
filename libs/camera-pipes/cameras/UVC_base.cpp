/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "UVC_base.hpp"

#include <spdlog/spdlog.h>

UVC_base::UVC_base()
{
  m_uvc = nullptr;
  m_dev = nullptr;
  m_dev_hndl = nullptr;
}

UVC_base::~UVC_base()
{
  close();
}

bool UVC_base::open()
{
  return open(0, 0);
}

bool UVC_base::open(const uint16_t vendor_id, const uint16_t product_id)
{
  uvc_error_t ret = uvc_init(&m_uvc, NULL);
  if (ret < 0) {
    uvc_perror(ret, "uvc_init");
    return false;
  }

  // HD Webcam C525
  /* filter devices: vendor_id, product_id, "serial_num" */
  // ret = uvc_find_device(m_uvc, &m_dev, 0x046d, 0x0826, NULL);

  // brio
  /* filter devices: vendor_id, product_id, "serial_num" */
  // ret = uvc_find_device(m_uvc, &m_dev, 0x046d, 0x085e, NULL);

  ret = uvc_find_device(m_uvc, &m_dev, vendor_id, product_id, NULL);
  if (ret < 0) {
    uvc_perror(ret, "uvc_find_device");
    return false;
  }

  ret = uvc_open(m_dev, &m_dev_hndl);
  if (ret < 0) {
    uvc_perror(ret, "uvc_open");
    return false;
  }

  return true;
}

bool UVC_base::close()
{
  if(m_dev_hndl)
  {
    uvc_close(m_dev_hndl);
    m_dev_hndl = nullptr;
  }
  if(m_dev)
  {
    uvc_unref_device(m_dev);
    m_dev = nullptr;
  }
  if(m_uvc)
  {
    uvc_exit(m_uvc);
    m_uvc = nullptr;
  }
  return true;
}

std::shared_ptr<uvc_frame_t> UVC_base::allocate_frame(const size_t data_bytes)
{
  uvc_frame_t* const frame = uvc_allocate_frame(data_bytes);
  
  if( ! frame )
  {
    SPDLOG_WARN("uvc_allocate_frame failed"); 
    return std::shared_ptr<uvc_frame_t>();
  }

  return std::shared_ptr<uvc_frame_t>(frame, &uvc_free_frame);
}

void UVC_base::copy_frame(const std::shared_ptr<uvc_frame_t>& src, std::shared_ptr<uvc_frame_t>& dest)
{
  if( ! src )
  {
    dest.reset();
  }
  else
  {
    if( ! dest )
    {
      dest = allocate_frame(src->data_bytes);
    }
    
    uvc_duplicate_frame(src.get(), dest.get());
  }
}

void UVC_base::register_callback(const std::function<void(uvc_frame_t* frame)>& cb)
{
  m_frame_cb = cb;
}
