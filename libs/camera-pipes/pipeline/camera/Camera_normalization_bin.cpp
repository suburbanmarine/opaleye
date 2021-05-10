#include "Camera_normalization_bin.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

Camera_normalization_bin::Camera_normalization_bin()
{
  
}

void Camera_normalization_bin::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}

bool Camera_normalization_bin::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool Camera_normalization_bin::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool Camera_normalization_bin::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue     = Gst::Queue::create();
    // m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    // m_in_queue->property_min_threshold_time()    = 0;
    // m_in_queue->property_min_threshold_buffers() = 0;
    // m_in_queue->property_min_threshold_bytes()   = 0;
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 1 * GST_SECOND;

    m_videorate    = Gst::ElementFactory::create_element("videorate");
    
    //caps
    m_out_caps = Gst::Caps::create_simple(
      "video/x-raw",
      // "format", Gst::Fourcc('M', 'J', 'P', 'G'),
      // "format", Gst::Fourcc(Glib::ustring("MJPG")),
      // "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "format", "I420",
      "framerate", Gst::Fraction(30, 1),
      "width",  640,
      "height", 480
      );

    m_out_capsfilter = Gst::CapsFilter::create();
    m_out_capsfilter->property_caps().set_value(m_src_caps);


    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_in_queue);
    m_bin->add(m_videorate);
    m_bin->add(m_out_capsfilter);
    m_bin->add(m_out_tee);
  }

  m_in_queue->link(m_videorate);
  m_videorate->link(m_out_capsfilter);
  m_out_capsfilter->link(m_out_tee);
  
  return true;
}

