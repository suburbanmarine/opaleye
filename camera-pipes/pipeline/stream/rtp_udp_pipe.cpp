#include "rtp_udp_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

rtp_udp_pipe::rtp_udp_pipe()
{
  
}

void rtp_udp_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool rtp_udp_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool rtp_udp_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}

bool rtp_udp_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue    = Gst::Queue::create();
    
    m_rtph264pay = Gst::ElementFactory::create_element("rtph264pay");
    m_rtph264pay->set_property("config-interval", -1);
    m_rtph264pay->set_property("pt", 96);

    m_udpsink = Gst::ElementFactory::create_element("udpsink");
    // m_udpsink->set_property("host", Glib::ustring("127.0.0.1"));
    m_udpsink->set_property("host", Glib::ustring("192.168.21.20"));
    m_udpsink->set_property("port", 50000);

    m_bin->add(m_in_queue);
    m_bin->add(m_rtph264pay);
    m_bin->add(m_udpsink);

    m_in_queue->link(m_rtph264pay);
    m_rtph264pay->link(m_udpsink);
  }

  return true;
}
