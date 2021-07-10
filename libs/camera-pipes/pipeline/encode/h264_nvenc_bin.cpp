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
    
    m_videoconvert = Gst::ElementFactory::create_element("nvvidconv");

    m_omxh264enc      = Gst::ElementFactory::create_element("nvv4l2h264enc");

    // 1 Baseline profile
    // 2 Main profile
    // 8 High profile
    m_omxh264enc->set_property("profile", 2);

    m_omxh264enc->set_property("bitrate",      2000000);
    m_omxh264enc->set_property("peak-bitrate", 3000000);

     // (0): variable_bitrate - GST_V4L2_VIDENC_VARIABLE_BITRATE
     // (1): constant_bitrate - GST_V4L2_VIDENC_CONSTANT_BITRATE
    m_omxh264enc->set_property("control-rate", 1);

   // (0): DisablePreset    - Disable HW-Preset
   // (1): UltraFastPreset  - UltraFastPreset for high perf
   // (2): FastPreset       - FastPreset
   // (3): MediumPreset     - MediumPreset
   // (4): SlowPreset       - SlowPreset
    m_omxh264enc->set_property("preset-level", 1);

    // m_omxh264enc->set_property("low-latency", 1);

    m_omxh264enc->set_property("insert-sps-pps", 1);
    // m_omxh264enc->set_property("insert-aud", true);
    m_omxh264enc->set_property("insert-vui", 1);

    m_omxh264enc->set_property("maxperf-enable", 1);

    m_h264parse = Gst::ElementFactory::create_element("h264parse");
    m_h264parse->set_property("config-interval", 1);

    // vbv-size

    //out caps
    m_out_caps = Gst::Caps::create_simple(
      "video/x-h264",
      "stream-format", "byte-stream",
      // "stream-format", "avc",
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
