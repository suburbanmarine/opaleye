#pragma once

#include "cameras/Logitech_brio.hpp"

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/appsrc.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

class Logitech_brio_pipe : public GST_element_base
{
public:
  Logitech_brio_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> back() override
  {
    return m_out_tee;
  }

  bool init(const char name[]) override;

  void new_frame_cb(const std::shared_ptr<uvc_frame_t>& frame);
#if 0
  void handle_need_data(guint val);
  void handle_enough_data();
  bool handle_seek_data(guint64 val);
#endif
protected:

  Glib::RefPtr<Gst::Bin>        m_bin;

  Glib::RefPtr<Gst::AppSrc>     m_src;
  Glib::RefPtr<Gst::Caps>       m_src_caps;
  Glib::RefPtr<Gst::Element>    m_jpegparse;
  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Caps>       m_out_caps;
  Glib::RefPtr<Gst::Tee>        m_out_tee;

  Logitech_brio m_camera;
  // std::shared_ptr<uvc_frame_t> m_frame_buffer;
};
