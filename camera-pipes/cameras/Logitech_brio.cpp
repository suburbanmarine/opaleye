#include "Logitech_brio.hpp"

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

Logitech_brio::Logitech_brio()
{

}

Logitech_brio::~Logitech_brio()
{
  close();
}

void Logitech_brio::dispatch_frame_cb(uvc_frame_t* frame_ptr, void* ctx)
{
  static_cast<Logitech_brio*>(ctx)->frame_cb(frame_ptr);
}

void Logitech_brio::frame_cb(uvc_frame_t* new_frame_ptr)
{
  uvc_duplicate_frame(new_frame_ptr, m_frame_buffer_back.get());

  {
    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
    std::swap(m_frame_buffer_back, m_frame_buffer_front);
  }

  if(m_frame_cb)
  {
    m_frame_cb(m_frame_buffer_front);
  }
}

bool Logitech_brio::open()
{
  //over allocate so we probably have enough for a 4k mjpeg
  m_frame_buffer_front = allocate_frame(4096*4096*3);
  m_frame_buffer_back  = allocate_frame(4096*4096*3);

  if( !(m_frame_buffer_front && m_frame_buffer_back) )
  {
    //allocation failed
    return false;
  }

  return UVC_base::open(0x046d, 0x085e);
}

bool Logitech_brio::start()
{
  uvc_error_t ret = uvc_get_stream_ctrl_format_size(
    m_dev_hndl,
    &m_ctrl,
    UVC_FRAME_FORMAT_MJPEG,
    640, 480, 30           /* width, height, fps */
    );
  
  if (ret < 0) {
    uvc_perror(ret, "uvc_get_stream_ctrl_format_size");
    return false;
  }

  // stream control block info
  uvc_print_stream_ctrl(&m_ctrl, stderr);

  // camera capabilities and config
  uvc_print_diag(m_dev_hndl, stderr);

  // UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY
  uvc_set_ae_mode(m_dev_hndl, 2);

  // TODO look at usb spec and figure out what these are
  // TODO AF mode
  uvc_set_focus_auto(m_dev_hndl, 0);

  ret = uvc_start_streaming(m_dev_hndl, &m_ctrl, Logitech_brio::dispatch_frame_cb, this, 0);
  if (ret < 0) {
    uvc_perror(ret, "uvc_get_stream_ctrl_format_size");
    return false;
  }

  return true;
}

bool Logitech_brio::stop()
{
  uvc_stop_streaming(m_dev_hndl);
  return true;
}

bool Logitech_brio::close()
{
  return UVC_base::close();
}

/// Allocate a new buffer and copy front to it
std::shared_ptr<uvc_frame_t> Logitech_brio::copy_front_buffer() const
{
  std::shared_ptr<uvc_frame_t> frame = allocate_frame(4096*4096*3);
  
  uvc_error_t ret = UVC_SUCCESS;
  {
    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
    ret = uvc_duplicate_frame(m_frame_buffer_front.get(), frame.get());
  }
  if(ret < 0)
  {
    uvc_perror(ret, "uvc_duplicate_frame");
    return std::shared_ptr<uvc_frame_t>();
  }

  return frame;
}

/// Copy front to provided buffer if it is allocated
bool Logitech_brio::copy_front_buffer(const std::shared_ptr<uvc_frame_t>& other) const
{
  if( ! other )
  {
    SPDLOG_ERROR("other is null");
    return false;
  }

  uvc_error_t ret = UVC_SUCCESS;
  {
    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
    ret = uvc_duplicate_frame(m_frame_buffer_front.get(), other.get());
  }
  if(ret < 0)
  {
    uvc_perror(ret, "uvc_duplicate_frame");
    return false;
  }

  return ret == UVC_SUCCESS;
}
