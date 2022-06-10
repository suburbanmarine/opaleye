/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "pipeline/camera/GST_v4l2_api.hpp"

#include "opaleye-util/errno_util.hpp"

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

class V4L2_webcam_pipe : public GST_v4l2_api
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

  void set_sensor_device(const char* dev_path);

  bool init(const char name[]) override;

//  void copy_frame(std::shared_ptr<uvc_frame_t>& out_frame) const
//  {
//    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
//    UVC_base::copy_frame(m_frame_buffer, out_frame);
//  }

protected:

  errno_util m_errno;

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
