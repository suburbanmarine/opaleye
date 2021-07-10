#include "rtp_h264_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

rtp_h264_pipe::rtp_h264_pipe()
{
  
}

void rtp_h264_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool rtp_h264_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool rtp_h264_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool rtp_h264_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue   = Gst::Queue::create();
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 1 * GST_SECOND;
    
    // m_in_queue->property_min_threshold_buffers() = 0;
    // m_in_queue->property_min_threshold_bytes()   = 0;
    // m_in_queue->property_min_threshold_time()    = 2 * GST_SECOND;

    m_rtph264pay = Gst::ElementFactory::create_element("rtph264pay");
    m_rtph264pay->set_property("config-interval", -1);
    m_rtph264pay->set_property("name", Glib::ustring("pay0"));
    m_rtph264pay->set_property("pt", 96);
    m_rtph264pay->set_property("aggregate-mode", 1);

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_in_queue);
    m_bin->add(m_rtph264pay);
    m_bin->add(m_out_tee);

    m_in_queue->link(m_rtph264pay);
    m_rtph264pay->link(m_out_tee);
    
  }

  return true;
}
