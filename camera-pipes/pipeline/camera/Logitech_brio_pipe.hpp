#pragma once

#include "cameras/Logitech_brio.hpp"

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/appsrc.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

#include <atomic>
#include <memory>
#include <mutex>

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

  void new_frame_cb(uvc_frame_t* frame);
  void handle_need_data(guint val);
  void handle_enough_data();
#if 0
  bool handle_seek_data(guint64 val);
#endif

  void copy_frame(std::shared_ptr<uvc_frame_t>& out_frame) const
  {
    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
    UVC_base::copy_frame(m_frame_buffer, out_frame);
  }

protected:

  bool on_bus_message(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message);

  Glib::RefPtr<Gst::Bin>        m_bin;
  Glib::RefPtr<Gst::Bus>        m_bus;

  Glib::RefPtr<Gst::AppSrc>     m_src;
  Glib::RefPtr<Gst::Caps>       m_src_caps;
  Glib::RefPtr<Gst::Element>    m_jpegparse;
  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Caps>       m_out_caps;
  Glib::RefPtr<Gst::Tee>        m_out_tee;

  std::atomic<bool> m_gst_need_data;

  Logitech_brio m_camera;

  mutable std::mutex m_frame_buffer_mutex;
  std::shared_ptr<uvc_frame_t> m_frame_buffer;
};
