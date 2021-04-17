#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/inputselector.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

class mux_pipe : public GST_element_base
{
public:
  mux_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_inputselector;
  }
  Glib::RefPtr<Gst::Element> back() override
  {
    return m_out_tee;
  }

  bool init(const char name[]) override;

  bool set_active_pad(const Glib::RefPtr<Gst::Pad>& pad);
  bool set_active_pad(const std::string& name);
  Glib::RefPtr<Gst::Pad> get_active_pad();

protected:

  Glib::RefPtr<Gst::Bin>           m_bin;

  // Glib::RefPtr<Gst::Queue>         m_in_queue;
  Glib::RefPtr<Gst::InputSelector> m_inputselector;
  Glib::RefPtr<Gst::Tee>           m_out_tee;

  std::map< std::string, Glib::RefPtr<Gst::Pad> > m_in_pads;
};
