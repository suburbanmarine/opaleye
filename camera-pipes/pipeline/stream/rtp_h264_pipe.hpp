#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/bin.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

class rtp_h264_pipe : public GST_element_base
{
public:
  rtp_h264_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  bool init(const char name[]) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }

protected:

  Glib::RefPtr<Gst::Bin>        m_bin;

  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Element>    m_rtph264pay; 
  Glib::RefPtr<Gst::Tee>        m_out_tee;
};
