#include "rtpsink_pipe.hpp"

#include <gstreamermm.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

rtpsink_pipe::rtpsink_pipe()
{
  
}

void rtpsink_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool rtpsink_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool rtpsink_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}

bool rtpsink_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue = Gst::Queue::create();
    
    m_rtpbin = Gst::ElementFactory::create_element("rtpbin");
    m_rtpbin->set_property("do-retransmission", false);

    
    //rtp in
    Glib::RefPtr<Gst::PadTemplate> send_rtp_sink_templ  = m_rtpbin->get_pad_template("send_rtp_sink_%u");
    Glib::RefPtr<Gst::Pad> send_rtp_sink_0  = m_rtpbin->request_pad(send_rtp_sink_templ,  "send_rtp_sink_0");

    //rtcp in
    Glib::RefPtr<Gst::PadTemplate> send_rtcp_sink_templ = m_rtpbin->get_pad_template("send_rtcp_sink_%u");
    Glib::RefPtr<Gst::Pad> send_rtcp_sink_0 = m_rtpbin->request_pad(send_rtcp_sink_templ, "send_rtcp_sink_0");
    
    //out
    Glib::RefPtr<Gst::PadTemplate> send_rtp_src_templ   = m_rtpbin->get_pad_template("send_rtp_src_%u");
    Glib::RefPtr<Gst::Pad> send_rtp_src_0  = m_rtpbin->request_pad(send_rtp_src_templ,"send_rtp_src_0");

    Glib::RefPtr<Gst::PadTemplate> send_rtcp_src_templ  = m_rtpbin->get_pad_template("send_rtcp_src_%u");
    Glib::RefPtr<Gst::Pad> send_rtcp_src_0 = m_rtpbin->request_pad(send_rtcp_src_templ, "send_rtcp_src_0");

    m_rtp_udpsink = Gst::ElementFactory::create_element("udpsink");
    m_rtp_udpsink->set_property("host", Glib::ustring("192.168.21.20"));
    m_rtp_udpsink->set_property("port", 50000);

    m_rtcp_udpsink = Gst::ElementFactory::create_element("udpsink");
    m_rtcp_udpsink->set_property("host",  Glib::ustring("192.168.21.20"));
    m_rtcp_udpsink->set_property("port",   50001);
    m_rtcp_udpsink->set_property("sync",  false);
    m_rtcp_udpsink->set_property("async", false);

    m_rtcp_udpsrc = Gst::ElementFactory::create_element("udpsrc");
    m_rtcp_udpsrc->set_property("port", 50002);

    if(! (m_rtp_udpsink && m_rtcp_udpsink && m_rtcp_udpsrc) )
    {
      SPDLOG_ERROR("Failed to init udp");
      return false;
    }

    if(! (send_rtp_sink_0 && send_rtcp_sink_0 && send_rtp_src_0 && send_rtcp_src_0) )
    {
      SPDLOG_ERROR("Failed to init rtp pads");
      return false;
    }

    // Glib::RefPtr<Gst::Pad> m_rtp_udpsink->get_static_pad("sink");
    // Glib::RefPtr<Gst::Pad> m_rtpc_udpsink->get_static_pad("sink");
    // Glib::RefPtr<Gst::Pad> m_rtpc_udpsrc->get_static_pad("src");

    m_bin->add(m_in_queue);
    m_bin->add(m_rtpbin);
    m_bin->add(m_rtp_udpsink);
    m_bin->add(m_rtcp_udpsink);
    m_bin->add(m_rtcp_udpsrc);

    //in
    m_in_queue->get_static_pad("src")->link(send_rtp_sink_0);
    m_rtcp_udpsrc->get_static_pad("src")->link(send_rtcp_sink_0);

    //out
    send_rtp_src_0->link(m_rtp_udpsink->get_static_pad("sink"));
    send_rtcp_src_0->link(m_rtcp_udpsink->get_static_pad("sink"));
    
  }

  return true;
}
