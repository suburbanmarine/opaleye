#pragma once

#include "cameras/Logitech_brio.hpp"

#include "pipeline/GST_element_base.hpp"

#include "errno_util.hpp"

#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <set>

class V4L2_webcam_pipe : public GST_element_base
{
public:
  V4L2_webcam_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> back() override
  {
    return m_out_tee;
  }

  bool init(const char name[]) override;

//  void copy_frame(std::shared_ptr<uvc_frame_t>& out_frame) const
//  {
//    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
//    UVC_base::copy_frame(m_frame_buffer, out_frame);
//  }

  bool set_exposure_mode();
  bool set_exposure_value();

  bool get_exposure_mode();
  bool get_exposure_value();

  bool v4l2_probe();

  bool get_property_description();
  
protected:

  std::map<uint32_t, v4l2_query_ext_ctrl> device_ctrl;
  std::map<uint32_t, std::map<int64_t, v4l2_querymenu>> menu_entries;

  errno_util m_errno;

  bool v4l2_ctrl_set(uint32_t id, const bool val);
  bool v4l2_ctrl_set(uint32_t id, const int32_t val);
  bool v4l2_ctrl_set(uint32_t id, const int64_t val);
  bool v4l2_ctrl_set(uint32_t id, const uint8_t val);
  bool v4l2_ctrl_set(uint32_t id, const uint16_t val);
  bool v4l2_ctrl_set(uint32_t id, const uint32_t val);

  bool v4l2_ctrl_get(uint32_t id, uint32_t which, bool*    const out_val);
  bool v4l2_ctrl_get(uint32_t id, uint32_t which, int32_t* const out_val);
  bool v4l2_ctrl_get(uint32_t id, uint32_t which, int64_t* const out_val);
  bool v4l2_ctrl_get(uint32_t id, uint32_t which, uint8_t* const out_val);
  bool v4l2_ctrl_get(uint32_t id, uint32_t which, uint16_t* const out_val);
  bool v4l2_ctrl_get(uint32_t id, uint32_t which, uint32_t* const out_val);

  bool v4l2_ctrl_set(v4l2_ext_control* const ctrl);
  bool v4l2_ctrl_get(uint32_t which, v4l2_ext_control* const ctrl);

  // bool on_bus_message(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message);
  void on_bus_message(const Glib::RefPtr<Gst::Message>& message);
  // void on_pad_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

  Glib::RefPtr<Gst::Bin>        m_bin;
  Glib::RefPtr<Gst::Bus>        m_bus;

  Glib::RefPtr<Gst::Element>    m_src;
  Glib::RefPtr<Gst::Caps>       m_src_caps;
  Glib::RefPtr<Gst::CapsFilter> m_in_capsfilter;
  Glib::RefPtr<Gst::Element>    m_jpegparse;
  Glib::RefPtr<Gst::Element>    m_videorate;
  Glib::RefPtr<Gst::Caps>       m_out_caps;
  Glib::RefPtr<Gst::CapsFilter> m_out_capsfilter;
  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Tee>        m_out_tee;
};
