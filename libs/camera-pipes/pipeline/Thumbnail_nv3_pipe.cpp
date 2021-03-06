/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "Thumbnail_nv3_pipe.hpp"

#include "pipeline/gst_common.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

Thumbnail_nv3_pipe::Thumbnail_nv3_pipe()
{
  
}

void Thumbnail_nv3_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}

bool Thumbnail_nv3_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}

bool Thumbnail_nv3_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    //queue
    m_in_queue = Gst::Queue::create();
    m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    m_in_queue->property_max_size_buffers()      = 2;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 0;

    m_videorate  = Gst::ElementFactory::create_element("videorate");

    m_videoconv = Gst::ElementFactory::create_element("nvvidconv");

    m_scale_queue = Gst::Queue::create();
    m_scale_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    m_scale_queue->property_max_size_buffers()      = 2;
    m_scale_queue->property_max_size_bytes()        = 0;
    m_scale_queue->property_max_size_time()         = 0;

    m_jpegenc    = Gst::ElementFactory::create_element("nvjpegenc");
    // m_jpegenc->set_property("idct-method",  GST::ifast);
    m_jpegenc->set_property("quality",  75);

    //out caps
    m_out_caps = Gst::Caps::create_simple(
      "image/jpeg",
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "framerate",          Gst::Fraction(1, 1),
      "width",              640,
      "height",             360
      );

    m_out_capsfilter = Gst::CapsFilter::create("outcaps");
    m_out_capsfilter->property_caps() = m_out_caps;
    //out caps
    m_appsink_caps = Gst::Caps::create_simple(
      "image/jpeg",
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "width",              640,
      "height",             360
      );

    m_appsink = Gst::AppSink::create();
    m_appsink->property_caps().set_value(m_appsink_caps);
    m_appsink->property_emit_signals() = true;
    m_appsink->property_drop()         = true;
    m_appsink->property_max_buffers()  = 1;
    m_appsink->property_sync()         = false;

    m_appsink->signal_new_sample().connect(
      [this]()
      {
        handle_new_sample();
        return Gst::FLOW_OK;
      }
    );

    m_bin->add(m_in_queue);
    m_bin->add(m_videorate);
    m_bin->add(m_videoconv);
    m_bin->add(m_scale_queue);
    m_bin->add(m_jpegenc);
    m_bin->add(m_out_capsfilter);
    m_bin->add(m_appsink);
  }

  m_in_queue->link(m_videorate);
  m_videorate->link(m_videoconv);
  m_videoconv->link(m_scale_queue);
  m_scale_queue->link(m_jpegenc);
  m_jpegenc->link(m_out_capsfilter);
  m_out_capsfilter->link(m_appsink);

  m_frame_buffer = std::make_shared<std::vector<uint8_t>>();
  m_frame_buffer->reserve(128U*1024U);

  return true;
}

void Thumbnail_nv3_pipe::handle_new_sample()
{
  Glib::RefPtr<Gst::Sample> sample = m_appsink->try_pull_sample(0);
  if(sample)
  {
    Glib::RefPtr<Gst::Buffer> buffer = sample->get_buffer();

    SPDLOG_INFO("Thumbnail_nv3_pipe::handle_new_sample has {}", buffer->get_size());
    {
      std::unique_lock<std::mutex> lock(m_frame_buffer_mutex);

      m_frame_buffer->resize(buffer->get_size());
      uint8_t* out_ptr = m_frame_buffer->data();

      guint num = buffer->n_memory();
      for(guint i = 0; i < num; i++)
      {
        Glib::RefPtr<Gst::Memory> mem_i = buffer->peek_memory(i);

        Gst::MapInfo map_info;
        mem_i->map(map_info, Gst::MAP_READ);

        SPDLOG_INFO("Thumbnail_nv3_pipe::handle_new_sample block {} is {}", i, map_info.get_size());

        guint8* blk_ptr = map_info.get_data();
        gsize   blk_len = map_info.get_size();

        std::copy_n(blk_ptr, blk_len, out_ptr);
        out_ptr += blk_len;

        mem_i->unmap(map_info);
      }
    }
  }
  else
  {
    SPDLOG_INFO("Thumbnail_nv3_pipe::handle_new_sample has null sample"); 
  }
}
