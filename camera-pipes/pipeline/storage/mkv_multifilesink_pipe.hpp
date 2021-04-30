#pragma once

#include "pipeline/GST_element_base.hpp"

#include "pipeline/GST_interpipesrc.hpp"

#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>

#include <atomic>
#include <mutex>
#include <condition_variable>

class mkv_multifilesink_pipe : public GST_element_base
{
public:
  mkv_multifilesink_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  // void remove_from_bin(const Glib::RefPtr<Gst::Bin>& bin) override;

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

  // void send_eos();

  // void install_wait_for_eos();
  // void wait_for_eos();
  // void remove_wait_for_eos();

protected:

  Gst::PadProbeReturn on_pad_probe(const Glib::RefPtr<Gst::Pad>& pad, const Gst::PadProbeInfo& probe_info);

  Glib::RefPtr<Gst::Bin>        m_bin;

  GST_interpipesrc              m_in_pipe;

  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Element>    m_matroskamux;
  Glib::RefPtr<Gst::Element>    m_multifilesink;

  // gulong                        m_eos_probe;
  
  // std::atomic<bool>             m_got_eos;
  // std::mutex                    m_mutex_got_eos;
  // std::condition_variable       m_cv_got_eos;

  std::string location;
  int index;
};
