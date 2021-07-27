#pragma once

#include "cameras/Logitech_brio.hpp"

#include "pipeline/GST_element_base.hpp"
#include "Thumbnail_sw_pipe.hpp"

#include "Stopwatch.hpp"

#include <spdlog/spdlog.h>

#include <gstreamermm/appsink.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/queue.h>

#include <atomic>
#include <memory>
#include <mutex>

/// Non-gstreamer thumbnail stream
class Thumbnail_sw2_pipe : public Thumbnail_pipe_base
{
public:
  Thumbnail_sw2_pipe();
  ~Thumbnail_sw2_pipe() override;

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }

  bool init(const char name[]) override;

  void handle_new_sample();

  void copy_frame_buffer(std::vector<uint8_t>* const out_frame_buffer)
  {
    {
      std::unique_lock<std::mutex> lock(m_thumb_jpeg_mutex);
      if(m_thumb_jpeg_buffer_front && out_frame_buffer)
      {
        out_frame_buffer->resize(m_thumb_jpeg_buffer_front->size());
        *out_frame_buffer = *m_thumb_jpeg_buffer_front;
      }
      else
      {
        out_frame_buffer->clear();
      }
    }
  }

protected:

  bool downsample_jpeg();

  Glib::RefPtr<Gst::Bin>        m_bin;

  Glib::RefPtr<Gst::Queue>      m_in_queue;

  Glib::RefPtr<Gst::Caps>       m_appsink_caps;
  Glib::RefPtr<Gst::AppSink>    m_appsink;

  Stopwatch m_stopwatch;

  //input jpeg from pipeline
  mutable std::mutex m_frame_buffer_mutex;
  std::shared_ptr<std::vector<uint8_t>> m_frame_buffer;

  //output scaled down jpeg
  mutable std::mutex m_thumb_jpeg_mutex;
  std::shared_ptr<std::vector<uint8_t>> m_thumb_jpeg_buffer_front;
  std::shared_ptr<std::vector<uint8_t>> m_thumb_jpeg_buffer_back;

};
