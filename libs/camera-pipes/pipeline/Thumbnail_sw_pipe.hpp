/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "cameras/Logitech_brio.hpp"

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/appsink.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

#include <atomic>
#include <memory>
#include <mutex>

class Thumbnail_pipe_base : public GST_element_base
{
public:

  Thumbnail_pipe_base()
  {

  }
  ~Thumbnail_pipe_base() override
  {
    
  }

  virtual void copy_frame_buffer(std::vector<uint8_t>* const out_frame_buffer) = 0;
};

class Thumbnail_sw_pipe : public Thumbnail_pipe_base
{
public:
  Thumbnail_sw_pipe();
  ~Thumbnail_sw_pipe() override;

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }

  bool init(const char name[]) override;

  void handle_new_sample();

  void copy_frame_buffer(std::vector<uint8_t>* const out_frame_buffer) override
  {
    {
      std::unique_lock<std::mutex> lock(m_frame_buffer_mutex);
      if(m_frame_buffer && out_frame_buffer)
      {
        *out_frame_buffer = *m_frame_buffer;
      }
      else
      {
        out_frame_buffer->clear();
      }
    }
  }

protected:

  Glib::RefPtr<Gst::Bin>        m_bin;

  Glib::RefPtr<Gst::Queue>      m_in_queue;
  
  Glib::RefPtr<Gst::Element>    m_videorate;

  Glib::RefPtr<Gst::Element>    m_videoscale;
  
  Glib::RefPtr<Gst::Queue>      m_scale_queue;

  Glib::RefPtr<Gst::Element>    m_jpegenc;

  Glib::RefPtr<Gst::Caps>       m_out_caps;
  Glib::RefPtr<Gst::CapsFilter> m_out_capsfilter;

  Glib::RefPtr<Gst::Caps>       m_appsink_caps;
  Glib::RefPtr<Gst::AppSink>    m_appsink;

  mutable std::mutex m_frame_buffer_mutex;
  std::shared_ptr<std::vector<uint8_t>> m_frame_buffer;
};
