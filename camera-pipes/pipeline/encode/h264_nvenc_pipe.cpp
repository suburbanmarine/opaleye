#include "h264_nvenc_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

h264_nvenc_pipe::h264_nvenc_pipe()
{
  
}

void h264_nvenc_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool h264_nvenc_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return false;
}
bool h264_nvenc_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool h264_nvenc_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue     = Gst::Queue::create();
    
    m_videoconvert = Gst::ElementFactory::create_element("videoconvert");

    m_omxh264enc      = Gst::ElementFactory::create_element("omxh264enc");

    // 1 Baseline profile
    // 2 Main profile
    // 8 High profile
    m_omxh264enc->set_property("profile", 2);

    m_omxh264enc->set_property("bitrate",      2000000);
    m_omxh264enc->set_property("peak-bitrate", 3000000);

    // 0 Disable
    // 1 Variable bit rate
    // 2 Constant bit rate
    // 3 Variable bit rate with frame skip. The encoder skips
    // frames as necessary to meet the target bit rate.
    // 4 Constant bit rate with frame skip
    m_omxh264enc->set_property("control-rate", 2);

    // 0 UltraFastPreset
    // 1 FastPreset
    // 2 MediumPreset
    // 3 SlowPreset
    m_omxh264enc->set_property("preset-level", 1);

    m_omxh264enc->set_property("low-latency", 1);

    m_omxh264enc->set_property("insert-sps-pps", 1);

    m_h264parse = Gst::ElementFactory::create_element("h264parse");

    //out caps
    m_out_caps = Gst::Caps::create_simple(
      "video/x-h264",
      // "stream-format", "byte-stream",
      "stream-format", "avc",
      "profile",       "main"
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