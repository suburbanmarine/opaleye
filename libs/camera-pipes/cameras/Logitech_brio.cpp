#include "Logitech_brio.hpp"

#include <boost/format.hpp>

#include <spdlog/spdlog.h>

Logitech_brio::Logitech_brio() : m_stream_on(false)
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
  if(m_frame_cb)
  {
    m_frame_cb(new_frame_ptr);
  }
}

bool Logitech_brio::open()
{
  if( ! UVC_base::open(0x046d, 0x085e) )
  {
    return false;
  }

  uvc_error_t ret = uvc_get_stream_ctrl_format_size(
    m_dev_hndl,
    &m_ctrl,
    UVC_FRAME_FORMAT_MJPEG,
    // 640, 480, 30           /* width, height, fps */
    // 1920, 1080, 30           /* width, height, fps */
    3840, 2160, 30           /* width, height, fps */
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

  return true;
}

bool Logitech_brio::start()
{
  if( ! m_stream_on )
  {
    uvc_error_t ret = uvc_start_streaming(m_dev_hndl, &m_ctrl, Logitech_brio::dispatch_frame_cb, this, 0);
    if (ret < 0) {
      uvc_perror(ret, "uvc_get_stream_ctrl_format_size");
      return false;
    }

    m_stream_on = true;
  }

  return true;
}

bool Logitech_brio::stop()
{
  if(m_stream_on)
  {
    uvc_stop_streaming(m_dev_hndl);
    m_stream_on = false;
  }
  return true;
}

bool Logitech_brio::close()
{
  return UVC_base::close();
}
