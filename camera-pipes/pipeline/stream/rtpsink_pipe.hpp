#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/bin.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>


#include <atomic>
#include <mutex>
#include <condition_variable>

class rtpsink_pipe : public GST_element_base
{
public:
  rtpsink_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  bool init(const char name[]) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }

void handle_pad_added(const Glib::RefPtr<Gst::Pad>& pad);

void handle_pad_removed(const Glib::RefPtr<Gst::Pad>& pad);

protected:

  bool wait_for_pad();

  Glib::RefPtr<Gst::Bin>        m_bin;

  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Element>    m_rtpbin; 

  std::atomic<bool> m_have_pad;
  std::mutex m_have_pad_mutex;
  std::condition_variable m_cond_var;
  Glib::RefPtr<Gst::Pad> m_new_pad;

  struct RTP_inst
  {
    //rtp from pipeline
    Glib::RefPtr<Gst::Pad> m_send_rtp_sink;
    
    //rtcp feedback from world
    // Glib::RefPtr<Gst::Element> m_rtcp_udpsrc;
    
    //rtp to world
    Glib::RefPtr<Gst::Queue>   m_rtp_queue;
    Glib::RefPtr<Gst::Element> m_rtp_udpsink;
    //rtcp to world
    Glib::RefPtr<Gst::Queue>   m_rtcp_queue;
    Glib::RefPtr<Gst::Element> m_rtcp_udpsink;
  };

  // std::map<unsigned, std::shared_ptr<RTP_inst>> m_rtp_conn;

  RTP_inst m_rtp_conn;
};
