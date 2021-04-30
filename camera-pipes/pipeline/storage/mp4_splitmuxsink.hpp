#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>

#include <atomic>
#include <mutex>
#include <condition_variable>

class mp4_splitmuxsink : public GST_element_base
{
public:
  mp4_splitmuxsink();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;

  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  bool unlink_front(const Glib::RefPtr<Gst::Element>& node) override;

  bool init(const char name[]) override;

  void set_location(const std::string& s);

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }

  Glib::RefPtr<Gst::Bin> get_bin()
  {
    return m_bin;
  }  

  void split_after_gop();
  void split_now();

  void handle_format_location();
  void handle_format_location_full();
  void handle_muxer_added();
  void handle_sink_added();

  // void send_eos();

  // void install_wait_for_eos();
  // void wait_for_eos();
  // void remove_wait_for_eos();

protected:

  Gst::PadProbeReturn on_pad_probe(const Glib::RefPtr<Gst::Pad>& pad, const Gst::PadProbeInfo& probe_info);

  Glib::RefPtr<Gst::Bin>        m_bin;

  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Element>    m_splitmuxsink;

  std::string location;
  int index;
};
