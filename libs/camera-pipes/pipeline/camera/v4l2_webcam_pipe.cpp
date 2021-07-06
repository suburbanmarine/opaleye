#include "v4l2_webcam_pipe.hpp"

#include "pipeline/gst_common.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

V4L2_webcam_pipe::V4L2_webcam_pipe()
{
  
}

void V4L2_webcam_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);

  m_bus = bin->get_bus();
}

bool V4L2_webcam_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool V4L2_webcam_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool V4L2_webcam_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    //source
    m_src = Gst::ElementFactory::create_element("v4l2src", name);
    m_src->set_property("do_timestamp", true);
    m_src->set_property("device", Glib::ustring("/dev/video5"));
    // m_src->add_probe(GST_PAD_PROBE_TYPE_IDLE | GST_PAD_PROBE_TYPE_EVENT_BOTH, sigc::mem_fun(&V4L2_webcam_pipe::on_pad_probe, this))

    //src caps
    m_src_caps = Gst::Caps::create_simple(
      "image/jpeg",
      // "format", Gst::Fourcc('M', 'J', 'P', 'G'),
      // "format", Gst::Fourcc(Glib::ustring("MJPG")),
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      // "format","MJPG",
      "framerate",          Gst::Fraction(30, 1),
      "width",              3840,
      "height",             2160
      );

    m_in_capsfilter = Gst::CapsFilter::create("incaps");
    m_in_capsfilter->property_caps() = m_src_caps;

    m_jpegparse    = Gst::ElementFactory::create_element("jpegparse");
    
    m_videorate    = Gst::ElementFactory::create_element("videorate");

    m_out_caps = Gst::Caps::create_simple(
      "image/jpeg",
      "format","UYVY",
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "framerate",         Gst::Fraction(30, 1)
      );

    m_out_capsfilter = Gst::CapsFilter::create("outcaps");
    m_out_capsfilter->property_caps() = m_out_caps;

    m_in_queue     = Gst::Queue::create();
    // m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    // m_in_queue->property_min_threshold_time()    = 0;
    // m_in_queue->property_min_threshold_buffers() = 0;
    // m_in_queue->property_min_threshold_bytes()   = 0;
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 1 * GST_SECOND;

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_src);
    m_bin->add(m_in_capsfilter);
    m_bin->add(m_jpegparse);
    m_bin->add(m_videorate);
    m_bin->add(m_out_capsfilter);
    m_bin->add(m_in_queue);
    m_bin->add(m_out_tee);
  }

  m_src->link(m_in_capsfilter);
  m_in_capsfilter->link(m_jpegparse);
  m_jpegparse->link(m_videorate);
  m_videorate->link(m_out_capsfilter);
  m_out_capsfilter->link(m_in_queue);
  m_in_queue->link(m_out_tee);
  
  // m_videoconvert->link(m_capsfilter);
  // m_capsfilter->link(m_out_queue);

  return true;
}
