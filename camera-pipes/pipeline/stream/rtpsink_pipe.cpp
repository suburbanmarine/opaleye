#include "rtpsink_pipe.hpp"

#include <gstreamermm.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

rtpsink_pipe::rtpsink_pipe() : m_have_pad(false)
{
  
}
rtpsink_pipe::~rtpsink_pipe()
{
    for(auto sigc : signal_handlers)
    {
        sigc.disconnect();
    }
    signal_handlers.clear();
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

void rtpsink_pipe::handle_pad_added(const Glib::RefPtr<Gst::Pad>& pad)
{
    // Glib::RefPtr<Gst::Pad> m_new_pad  = pad;
    m_have_pad = true;
    m_cond_var.notify_one();

    Glib::ustring pad_name = pad->get_name();
    Glib::RefPtr<Gst::Element> pad_parent = pad->get_parent_element();
    SPDLOG_ERROR("handle_pad_added {:s}/{:s}", pad_parent->get_name().c_str(), pad_name.c_str());

    // Glib::RefPtr<Gst::Element> m_rtcp_udpsrc_0  = m_rtp_conn[0]->m_rtcp_udpsrc;

    //connection 0
    // m_rtp_conn[0] = std::make_shared<RTP_inst>();
    // m_rtp_conn[0]->m_send_rtp_sink = send_rtp_sink_0;

    // Glib::RefPtr<Gst::Element> m_rtp_udpsink_0 = Gst::ElementFactory::create_element("udpsink");
    // m_rtp_udpsink_0->set_property("host", Glib::ustring("192.168.21.20"));
    // m_rtp_udpsink_0->set_property("port", 50000);
    // m_rtp_conn[0]->m_rtp_udpsink = m_rtp_udpsink_0;

    // Glib::RefPtr<Gst::Element> m_rtcp_udpsink_0 = Gst::ElementFactory::create_element("udpsink");
    // m_rtcp_udpsink_0->set_property("host",  Glib::ustring("192.168.21.20"));
    // m_rtcp_udpsink_0->set_property("port",   50001);
    // m_rtcp_udpsink_0->set_property("sync",  false);
    // m_rtcp_udpsink_0->set_property("async", false);
    // m_rtp_conn[0]->m_rtcp_udpsink = m_rtcp_udpsink_0;

    // m_rtp_conn[0]->m_rtcp_udpsrc = Gst::ElementFactory::create_element("udpsrc");
    // m_rtp_conn[0]->m_rtcp_udpsrc->set_property("port", 50002);

    // m_bin->add(m_rtcp_udpsrc);
    // m_bin->add(m_rtp_udpsink_0);
    // m_bin->add(m_rtcp_udpsink_0);

    // rtcp in
    // Glib::RefPtr<Gst::PadTemplate> send_rtcp_sink_templ = m_rtpbin->get_pad_template("send_rtcp_sink_%u");
    // Glib::RefPtr<Gst::Pad> send_rtcp_sink_0 = m_rtpbin->request_pad(send_rtcp_sink_templ, "send_rtcp_sink_0");
    
    // rtp out
    // Glib::RefPtr<Gst::PadTemplate> send_rtp_src_templ   = m_rtpbin->get_pad_template("send_rtp_src_%u");
    // Glib::RefPtr<Gst::Pad> send_rtp_src_0  = m_rtpbin->request_pad(send_rtp_src_templ,"send_rtp_src_0");

    // rtcp out
    // Glib::RefPtr<Gst::PadTemplate> send_rtcp_src_templ  = m_rtpbin->get_pad_template("send_rtcp_src_%u");
    // Glib::RefPtr<Gst::Pad> send_rtcp_src_0 = m_rtpbin->request_pad(send_rtcp_src_templ, "send_rtcp_src_0");

    //in
    // m_rtcp_udpsrc_0->get_static_pad("src")->link(send_rtcp_sink_0);

    //out
    // send_rtp_src_0->link(m_rtp_udpsink_0->get_static_pad("sink"));
    // send_rtcp_src_0->link(m_rtcp_udpsink_0->get_static_pad("sink"));
}

void rtpsink_pipe::handle_pad_removed(const Glib::RefPtr<Gst::Pad>& pad)
{
    SPDLOG_ERROR("handle_pad_removed");
}

bool rtpsink_pipe::wait_for_pad()
{
    std::unique_lock<std::mutex> lock(m_have_pad_mutex);
    return m_cond_var.wait_for(lock, std::chrono::milliseconds(5000), [this]{return m_have_pad.load();});
}

bool rtpsink_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue = Gst::Queue::create();
    // m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    m_in_queue->property_max_size_buffers() = 0;
    m_in_queue->property_max_size_bytes()   = 0;
    m_in_queue->property_max_size_time()    = 1 * GST_SECOND;
    
    // m_in_queue->property_min_threshold_buffers() = 0;
    // m_in_queue->property_min_threshold_bytes()   = 0;
    // m_in_queue->property_min_threshold_time()    = 2 * GST_SECOND;

    m_rtpbin = Gst::ElementFactory::create_element("rtpbin");
    m_rtpbin->set_property("do-retransmission", false);

    sigc::connection pad_add_conn = m_rtpbin->signal_pad_added().connect(
      [this](const Glib::RefPtr<Gst::Pad>& pad){handle_pad_added(pad);}
      );
    signal_handlers.push_back(pad_add_conn);

    sigc::connection pad_rm_conn = m_rtpbin->signal_pad_removed().connect(
      [this](const Glib::RefPtr<Gst::Pad>& pad){handle_pad_removed(pad);}
      );
    signal_handlers.push_back(pad_rm_conn);

    //wait for each pad
    m_have_pad = false;
    Glib::RefPtr<Gst::PadTemplate> send_rtp_sink_templ  = m_rtpbin->get_pad_template("send_rtp_sink_%u");
    Glib::RefPtr<Gst::Pad> send_rtp_sink_0  = m_rtpbin->request_pad(send_rtp_sink_templ, "send_rtp_sink_0");
    if( ! wait_for_pad() )
    {
        SPDLOG_ERROR("Could not get pad");
        return false;
    }
    m_rtp_conn.m_send_rtp_sink = send_rtp_sink_0;
   
    // m_have_pad = false;
    // Glib::RefPtr<Gst::PadTemplate> send_rtp_src_templ   = m_rtpbin->get_pad_template("send_rtp_src_%u");
    // Glib::RefPtr<Gst::Pad> send_rtp_src_0  = m_rtpbin->request_pad(send_rtp_src_templ,"send_rtp_src_0");
    // if( ! wait_for_pad() )
    // {
    //     SPDLOG_ERROR("Could not get pad");
    //     return false;
    // }

    // m_have_pad = false;
    // Glib::RefPtr<Gst::PadTemplate> send_rtcp_src_templ  = m_rtpbin->get_pad_template("send_rtcp_src_%u");
    // Glib::RefPtr<Gst::Pad> send_rtcp_src_0 = m_rtpbin->request_pad(send_rtcp_src_templ, "send_rtcp_src_0");
    // if( ! wait_for_pad() )
    // {
    //     SPDLOG_ERROR("Could not get pad");
    //     return false;
    // }

    m_rtp_conn.m_multiudpsink = std::make_shared<multiudpsink_pipe>();
    if( ! (m_rtp_conn.m_multiudpsink && m_rtp_conn.m_multiudpsink->init("multiudpsink")) )
    {
        SPDLOG_ERROR("Could not init multiudpsink");
        return false;
    }

    m_bin->add(m_in_queue);
    m_bin->add(m_rtpbin);
    m_rtp_conn.m_multiudpsink->add_to_bin(m_bin);

    //in
    // m_rtp_conn[0]->m_send_rtp_sink = send_rtp_sink_0;
    m_in_queue->get_static_pad("src")->link(send_rtp_sink_0);

    //out
    Glib::RefPtr<Gst::Pad> send_rtp_src_0 = m_rtpbin->get_static_pad("send_rtp_src_0");
    m_rtp_conn.m_multiudpsink->link_front(send_rtp_src_0);

    if( ! m_rtp_conn.m_multiudpsink->add_client("127.0.0.1", 5000) )
    {
        SPDLOG_ERROR("Could not start stream");
        return false;   
    }
    if( ! m_rtp_conn.m_multiudpsink->add_client("127.0.0.1", 5008) )
    {
        SPDLOG_ERROR("Could not start stream");
        return false;
    }
  }

  return true;
}
