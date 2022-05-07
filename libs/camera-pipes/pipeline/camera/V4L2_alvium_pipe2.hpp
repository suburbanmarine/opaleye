/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "opaleye-util/thread_base.hpp"

#include "pipeline/camera/GST_v4l2_api.hpp"

#include "cameras/Alvium_v4l2.hpp"

#include <gstreamermm/appsrc.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>
#include <gstreamermm/fakesink.h>

class V4L2_alvium_pipe2 : public GST_v4l2_api
{
  //friend V4L2_alvium_gst_worker;
public:
  V4L2_alvium_pipe2();
  ~V4L2_alvium_pipe2() override;

  void set_framebuffer_callback(const GST_camera_base::FramebufferCallback& cb) override
  {
    std::unique_lock<std::mutex> lock(m_frame_buffer_mutex);
    m_buffer_dispatch_cb = cb;
  }

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  // Glib::RefPtr<Gst::Element> back() override
  // {
  //   return m_out_tee;
  // }

  bool init(const char name[]) override;

protected:
  bool close();

  void handle_need_data(guint val);
  void handle_enough_data();

  void new_frame_cb_XR24(); // 32-bit BGRX 8-8-8-8

  std::thread m_thread;

  std::atomic<bool> m_gst_need_data;
  std::mutex        m_frame_buffer_mutex;
  std::shared_ptr<std::vector<uint8_t>> m_frame_buffer;
  FramebufferCallback m_buffer_dispatch_cb;

  Glib::RefPtr<Gst::Bin>        m_bin;
  Glib::RefPtr<Gst::Bus>        m_bus;

  Glib::RefPtr<Gst::AppSrc>     m_src;
  Glib::RefPtr<Gst::Caps>       m_src_caps;
  Glib::RefPtr<Gst::Element>    m_sink;
  


  // std::chrono::nanoseconds m_curr_pts;
};
