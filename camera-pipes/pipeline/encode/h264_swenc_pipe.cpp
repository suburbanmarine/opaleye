#include "h264_swenc_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

h264_swenc_pipe::h264_swenc_pipe()
{
  
}

void h264_swenc_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool h264_swenc_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return false;
}
bool h264_swenc_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool h264_swenc_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue     = Gst::Queue::create();
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 10 * GST_SECOND;

    // m_in_queue->property_min_threshold_buffers() = 0;
    // m_in_queue->property_min_threshold_bytes()   = 0;
    // m_in_queue->property_min_threshold_time()    = 2 * GST_SECOND;

    m_x264enc      = Gst::ElementFactory::create_element("x264enc");

    m_x264enc->set_property("config-interval", 10);

    m_x264enc->set_property("insert-vui", 1);

    m_x264enc->set_property("bitrate", 3000);

    // cbr (0) – Constant Bitrate Encoding
    // quant (4) – Constant Quantizer
    // qual (5) – Constant Quality
    // pass1 (17) – VBR Encoding - Pass 1
    // pass2 (18) – VBR Encoding - Pass 2
    // pass3 (19) – VBR Encoding - Pass 3
    m_x264enc->set_property("pass", 0);

    // None (0) – No preset
    // ultrafast (1) – ultrafast
    // superfast (2) – superfast
    // veryfast (3) – veryfast
    // faster (4) – faster
    // fast (5) – fast
    // medium (6) – medium
    // slow (7) – slow
    // slower (8) – slower
    // veryslow (9) – veryslow
    // placebo (10) – placebo
    m_x264enc->set_property("speed-preset", 1);

    // stillimage (0x00000001) – Still image
    // fastdecode (0x00000002) – Fast decode
    // zerolatency (0x00000004) – Zero latency
    m_x264enc->set_property("tune", 4);



    m_x264enc->set_property("bframes", 0);
    m_x264enc->set_property("mb-tree", false);
    m_x264enc->set_property("sliced-threads", true);
    m_x264enc->set_property("sync-lookahead", 0);
    m_x264enc->set_property("rc-lookahead", 0);


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

    // m_out_queue     = Gst::Queue::create();
    // m_out_queue->property_max_size_buffers()      = 0;
    // m_out_queue->property_max_size_bytes()        = 0;
    // m_out_queue->property_max_size_time()         = 10 * GST_SECOND;
    // m_out_queue->property_min_threshold_buffers() = 0;
    // m_out_queue->property_min_threshold_bytes()   = 0;
    // m_out_queue->property_min_threshold_time()    = 2 * GST_SECOND;

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_in_queue);
    m_bin->add(m_x264enc);
    m_bin->add(m_h264parse);
    m_bin->add(m_capsfilter);
    // m_bin->add(m_out_queue);
    m_bin->add(m_out_tee);

    m_in_queue->link(m_x264enc);
    m_x264enc->link(m_h264parse);
    m_h264parse->link(m_capsfilter);
    m_capsfilter->link(m_out_tee);
    // m_out_queue->link(m_out_tee);
  }

  return true;
}
