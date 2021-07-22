#include "rtp_mjpeg_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

rtp_mjpeg_pipe::rtp_mjpeg_pipe()
{
  
}

void rtp_mjpeg_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool rtp_mjpeg_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool rtp_mjpeg_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool rtp_mjpeg_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue   = Gst::Queue::create();
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 1 * GST_SECOND;
    
    m_rtpmjpegpay = Gst::ElementFactory::create_element("rtpjpegpay");
    m_rtpmjpegpay->set_property("name", Glib::ustring("pay0"));
    m_rtpmjpegpay->set_property("encoding-name", Glib::ustring("JPEG"));
    m_rtpmjpegpay->set_property("pt", 26);

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_in_queue);
    m_bin->add(m_rtpmjpegpay);
    m_bin->add(m_out_tee);

    m_in_queue->link(m_rtpmjpegpay);
    m_rtpmjpegpay->link(m_out_tee);
    
  }

  return true;
}
