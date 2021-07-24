#pragma once

#include "cameras/Logitech_brio.hpp"

#include "pipeline/GST_element_base.hpp"
#include "Thumbnail_sw_pipe.hpp"

#include <gstreamermm/appsink.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

#include <atomic>
#include <memory>
#include <mutex>

/// NV accelerated but non-gstreamer thumbnail stream
class Thumbnail_nv2_pipe : public Thumbnail_pipe_base
{
public:
  Thumbnail_nv2_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override
  {
    return false;
  }

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
        *out_frame_buffer = *m_thumb_jpeg_buffer_front;
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

  std::shared_ptr<NvJPEGDecoder> m_jpegdec;
  std::shared_ptr<NvJPEGEncoder> m_jpegenc;

  //input jpeg from pipeline
  mutable std::mutex m_frame_buffer_mutex;
  std::shared_ptr<std::vector<uint8_t>> m_frame_buffer;

  //output scaled down jpeg
  mutable std::mutex m_thumb_jpeg_mutex;
  std::shared_ptr<std::vector<uint8_t>> m_thumb_jpeg_buffer_front;
  std::shared_ptr<std::vector<uint8_t>> m_thumb_jpeg_buffer_back;

};
