#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/valve.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

class valve_pipe : public GST_element_base
{
public:
  valve_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_valve;
  }
  Glib::RefPtr<Gst::Element> back() override
  {
    return m_out_tee;
  }

  bool init(const char name[]) override;

  void set_drop(const bool drop);

protected:

  Glib::RefPtr<Gst::Bin>           m_bin;

  // Glib::RefPtr<Gst::Queue>         m_in_queue;
  Glib::RefPtr<Gst::Valve> m_valve;
  Glib::RefPtr<Gst::Tee>   m_out_tee;
};
