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

class V4L2_alvium_pipe;

class V4L2_alvium_frame_worker : public thread_base
{
public:

  V4L2_alvium_frame_worker(const Alvium_v4l2::FrameCallback& cb, const std::shared_ptr<Alvium_v4l2>& cam)
  {
    m_cb  = cb;
    m_cam = cam;
  }

  void work() override;

protected:
  std::shared_ptr<Alvium_v4l2> m_cam;
  Alvium_v4l2::FrameCallback   m_cb;
};
/*
class V4L2_alvium_gst_worker : public thread_base
{
public:

  V4L2_alvium_gst_worker(V4L2_alvium_pipe* cam_pipe)
  {
    m_cam_pipe = cam_pipe;
  }

  void work() override;

protected:
  V4L2_alvium_pipe* m_cam_pipe;
};
*/
class V4L2_alvium_pipe : public GST_v4l2_api
{
  //friend V4L2_alvium_gst_worker;
public:
  V4L2_alvium_pipe();
  ~V4L2_alvium_pipe() override
  {
    close();
  }

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

protected:
  bool close();
  //avt specific things
  std::shared_ptr<Alvium_v4l2> m_cam;
  std::string m_dev_path;
  uint32_t    m_fourcc;
  std::string m_trigger_mode;

  void handle_need_data(guint val);
  void handle_enough_data();

  void new_frame_cb_JXR0(const Alvium_v4l2::ConstMmapFramePtr& frame_buf); // 10-bit/16-bit Bayer RGRG/GBGB
  void new_frame_cb_JXR2(const Alvium_v4l2::ConstMmapFramePtr& frame_buf); // 12-bit/16-bit Bayer RGRG/GBGB
  void new_frame_cb_JXY2(const Alvium_v4l2::ConstMmapFramePtr& frame_buf); // 12-bit/16-bit Greyscale
  void new_frame_cb_XR24(const Alvium_v4l2::ConstMmapFramePtr& frame_buf); // 32-bit BGRX 8-8-8-8

  std::shared_ptr<V4L2_alvium_frame_worker> m_frame_worker;

  std::atomic<bool> m_gst_need_data;
  std::mutex        m_frame_buffer_mutex;
  std::shared_ptr<std::vector<uint8_t>> m_frame_buffer;
  FramebufferCallback m_buffer_dispatch_cb;

  Glib::RefPtr<Gst::Bin>        m_bin;
  Glib::RefPtr<Gst::Bus>        m_bus;

  Glib::RefPtr<Gst::AppSrc>     m_src;
  Glib::RefPtr<Gst::Caps>       m_src_caps;
  // Glib::RefPtr<Gst::Element>    m_videoconvert;
  // Glib::RefPtr<Gst::Element>    m_videorate;
  // Glib::RefPtr<Gst::Caps>       m_out_caps;
  // Glib::RefPtr<Gst::CapsFilter> m_out_capsfilter;
  // Glib::RefPtr<Gst::Queue>      m_in_queue;
  // Glib::RefPtr<Gst::Tee>        m_out_tee;
  Glib::RefPtr<Gst::FakeSink>   m_sink;
  


  // std::chrono::nanoseconds m_curr_pts;
};
