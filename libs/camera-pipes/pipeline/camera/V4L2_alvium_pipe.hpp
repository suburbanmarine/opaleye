/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "opaleye-util/thread_base.hpp"

#include "pipeline/GST_element_base.hpp"

#include "cameras/Alvium_v4l2.hpp"

#include <gstreamermm/appsrc.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

class V4L2_alvium_frame_worker : public thread_base
{
public:

  V4L2_alvium_frame_worker(const Alvium_v4l2::FrameCallback& cb, const std::shared_ptr<Alvium_v4l2>& cam)
  {
    m_cb = cb;
    m_cam = cam;
  }

  void work() override
  {
    while( ! is_interrupted() )
    {
      m_cam->wait_for_frame(std::chrono::milliseconds(250), m_cb);
    }
  }

protected:
  std::shared_ptr<Alvium_v4l2> m_cam;
  Alvium_v4l2::FrameCallback   m_cb;
};

class V4L2_alvium_pipe : public GST_element_base
{
public:
  V4L2_alvium_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  // Glib::RefPtr<Gst::Element> back() override
  // {
  //   return m_out_tee;
  // }

  void reset();

  void set_sensor_device(const char* dev_path);
  bool open(const char dev_path[]);
  bool close();

  bool init(const char name[]) override;

protected:
  std::shared_ptr<Alvium_v4l2> m_cam;

  void handle_need_data(guint val);
  void handle_enough_data();

  void new_frame_cb_JXR0(const Alvium_v4l2::ConstMmapFramePtr& frame_buf);
  void new_frame_cb_JXR2(const Alvium_v4l2::ConstMmapFramePtr& frame_buf);
  void new_frame_cb_JXY2(const Alvium_v4l2::ConstMmapFramePtr& frame_buf);
  void new_frame_cb_XR24(const Alvium_v4l2::ConstMmapFramePtr& frame_buf);

  std::shared_ptr<V4L2_alvium_frame_worker> m_frame_worker;

  std::atomic<bool> m_gst_need_data;
  std::mutex        m_frame_buffer_mutex;
  std::shared_ptr<std::vector<uint8_t>> m_frame_buffer; 

  Glib::RefPtr<Gst::Bin>        m_bin;
  Glib::RefPtr<Gst::Bus>        m_bus;

  Glib::RefPtr<Gst::AppSrc>     m_src;
  Glib::RefPtr<Gst::Caps>       m_src_caps;
  Glib::RefPtr<Gst::Element>    m_videorate;
  Glib::RefPtr<Gst::Caps>       m_out_caps;
  Glib::RefPtr<Gst::CapsFilter> m_out_capsfilter;
  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Tee>        m_out_tee;

};
