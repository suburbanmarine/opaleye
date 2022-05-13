/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "h264_nvenc_bin.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

h264_nvenc_bin::h264_nvenc_bin()
{
  
}

void h264_nvenc_bin::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool h264_nvenc_bin::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return false;
}
bool h264_nvenc_bin::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool h264_nvenc_bin::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue     = Gst::Queue::create();
    m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    m_in_queue->property_max_size_buffers() = 10;
    m_in_queue->property_max_size_bytes()   = 0;
    m_in_queue->property_max_size_time()    = 0;

    m_videoconvert = Gst::ElementFactory::create_element("nvvidconv");

    m_omxh264enc      = Gst::ElementFactory::create_element("omxh264enc");

    // 1 Baseline profile
    // 2 Main profile
    // 8 High profile
    m_omxh264enc->set_property("profile", 2);

    m_omxh264enc->set_property("bitrate",      10*1000*1000);
    m_omxh264enc->set_property("peak-bitrate", 12*1000*1000);

    // 0 Disable
    // 1 Variable bit rate
    // 2 Constant bit rate
    // 3 Variable bit rate with frame skip. The encoder skips
    // frames as necessary to meet the target bit rate.
    // 4 Constant bit rate with frame skip
    m_omxh264enc->set_property("control-rate", 2);

    // 0 Disable
    // 1 Drop 1 in 5 frames
    // 2 Drop 1 in 3 frames
    // 3 Drop 1 in 2 frames
    // 4 Drop 2 in 3 frames
    // m_omxh264enc->set_property("temporal-tradeoff", 0);

    // 0 UltraFastPreset
    // 1 FastPreset
    // 2 MediumPreset
    // 3 SlowPreset
    m_omxh264enc->set_property("preset-level", 0);

    // m_omxh264enc->set_property("low-latency", 1);

    m_omxh264enc->set_property("insert-sps-pps", 1);
    // m_omxh264enc->set_property("insert-aud", true);
    m_omxh264enc->set_property("insert-vui", 1);

    m_h264parse = Gst::ElementFactory::create_element("h264parse");
    // m_h264parse->set_property("config-interval", 1);

    // vbv-size

    //out caps
    m_out_caps = Gst::Caps::create_simple(
      "video/x-h264",
      "stream-format", "avc",
      "profile",       "main",
      "width",  1920,
      "height", 1080
      );

    //out caps filter
    m_capsfilter = Gst::CapsFilter::create();
    m_capsfilter->property_caps().set_value(m_out_caps);

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_in_queue);
    m_bin->add(m_videoconvert);
    m_bin->add(m_omxh264enc);
    m_bin->add(m_h264parse);
    m_bin->add(m_capsfilter);
    m_bin->add(m_out_tee);

    m_in_queue->link(m_videoconvert);
    m_videoconvert->link(m_omxh264enc);
    m_omxh264enc->link(m_h264parse);
    m_h264parse->link(m_capsfilter);
    m_capsfilter->link(m_out_tee);
  }

  return true;
}
