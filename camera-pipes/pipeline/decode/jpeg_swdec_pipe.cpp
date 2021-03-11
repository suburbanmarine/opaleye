#include "jpeg_swdec_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

jpeg_swdec_pipe::jpeg_swdec_pipe()
{
  
}

void jpeg_swdec_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool jpeg_swdec_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return false;
}
bool jpeg_swdec_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool jpeg_swdec_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue     = Gst::Queue::create();
    
    m_jpegdec = Gst::ElementFactory::create_element("jpegdec");

    //out caps
    m_out_caps = Gst::Caps::create_simple(
      "video/x-raw"
      // "format", "RGB"
      );

    //out caps filter
    m_capsfilter = Gst::CapsFilter::create();
    m_capsfilter->property_caps().set_value(m_out_caps);

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_in_queue);
    m_bin->add(m_jpegdec);
    m_bin->add(m_capsfilter);
    m_bin->add(m_out_tee);

    m_in_queue->link(m_jpegdec);
    m_jpegdec->link(m_capsfilter);
    m_capsfilter->link(m_out_tee);
  }

  return true;
}
