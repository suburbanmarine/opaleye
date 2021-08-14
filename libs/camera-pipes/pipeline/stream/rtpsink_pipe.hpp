/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/bin.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>
#include "multiudpsink_pipe.hpp"

#include <atomic>
#include <mutex>
#include <condition_variable>

class rtpsink_pipe : public GST_element_base
{
public:
  rtpsink_pipe();
  ~rtpsink_pipe() override;

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

  // bool add_rtp_stream(const std::string& dest, const uint16_t port);
  // bool remove_rtp_stream(const std::string& dest, const uint16_t port);

  bool add_udp_client(const std::string& dest, const uint16_t port);
  bool remove_udp_client(const std::string& dest, const uint16_t port);

protected:

  bool wait_for_pad();

  Glib::RefPtr<Gst::Bin>        m_bin;

  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Element>    m_rtpbin; 

  std::atomic<bool> m_have_pad;
  std::mutex m_have_pad_mutex;
  std::condition_variable m_cond_var;

  struct RTP_inst
  {
    //rtp from pipeline
    Glib::RefPtr<Gst::Pad> m_send_rtp_sink;
    
    //rtcp feedback from world
    // Glib::RefPtr<Gst::Element> m_rtcp_udpsrc;
    
    //rtp to world
    std::shared_ptr<multiudpsink_pipe> m_multiudpsink;

    //rtcp to world
    // Glib::RefPtr<Gst::Queue>   m_rtcp_queue;
    // Glib::RefPtr<Gst::Element> m_rtcp_udpsink;
  };

  // std::map<unsigned, std::shared_ptr<RTP_inst>> m_rtp_conn;

  RTP_inst m_rtp_conn;

  std::list<sigc::connection> signal_handlers;
};
