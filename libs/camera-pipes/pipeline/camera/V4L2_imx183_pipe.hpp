/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "opaleye-util/thread_base.hpp"

#include "pipeline/camera/GST_v4l2_api.hpp"

#include "cameras/imx183_v4l2.hpp"

#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

class V4L2_imx183_pipe;

class V4L2_imx183_frame_worker : public thread_base
{
public:

  V4L2_imx183_frame_worker(const v4l2_base::FrameCallback& cb, const std::shared_ptr<imx183_v4l2>& cam)
  {
    m_cb  = cb;
    m_cam = cam;
  }

  void work() override;

protected:
  std::shared_ptr<imx183_v4l2> m_cam;
  v4l2_base::FrameCallback   m_cb;
};

class V4L2_imx183_pipe : public GST_v4l2_api
{
  //friend V4L2_alvium_gst_worker;
public:
  V4L2_imx183_pipe();
  ~V4L2_imx183_pipe() override;

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

  void set_params(const char dev_path[], const uint32_t fourcc, const std::string& trigger_mode);
  bool init(const char name[]) override;

  //for api calls
  bool set_camera_property(const std::string& property_id, const std::string& value) override;
  bool start_streaming();
  bool stop_streaming();
  bool set_trigger_mode(const std::string& mode);

  static constexpr uint32_t PIX_FMT_RG12 = v4l2_fourcc('R','G','1','2'); // 12-bit Bayer RGGB

protected:
  bool close();
  //avt specific things
  std::shared_ptr<imx183_v4l2> m_cam;
  std::string m_dev_path;
  uint32_t    m_fourcc;
  std::string m_trigger_mode;

  void handle_need_data(guint val);
  void handle_enough_data();

  void new_frame_cb_RG12(const v4l2_base::ConstMmapFramePtr& frame_buf); // 8-bit Bayer RGRG/GBGB

  std::shared_ptr<V4L2_imx183_frame_worker> m_frame_worker;

  std::atomic<bool> m_gst_need_data;
  std::mutex        m_frame_buffer_mutex;
  std::shared_ptr<std::vector<uint8_t>> m_frame_buffer;
  FramebufferCallback m_buffer_dispatch_cb;
  std::chrono::nanoseconds m_curr_pts;

  Glib::RefPtr<Gst::Bin>        m_bin;
  Glib::RefPtr<Gst::Bus>        m_bus;

  GstElement*                   m_appsrc;
  Glib::RefPtr<Gst::Queue>      m_in_queue;
  GstCaps*                      m_src_caps;

  Glib::RefPtr<Gst::Element>    m_videoconvert;
  Glib::RefPtr<Gst::Caps>       m_out_caps;
  Glib::RefPtr<Gst::CapsFilter> m_out_capsfilter;
  Glib::RefPtr<Gst::Queue>      m_out_queue;
  Glib::RefPtr<Gst::Tee>        m_out_tee;
};
