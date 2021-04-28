#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/inputselector.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

class GST_interpipesrc : public GST_element_base
{
public:
  GST_interpipesrc();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> back() override
  {
    return m_out_tee;
  }

  bool init(const char name[]) override;

  void set_listen_to(const char name[]);

protected:

  Glib::RefPtr<Gst::Bin>     m_bin;

  Glib::RefPtr<Gst::Element> m_interpipesrc;
  Glib::RefPtr<Gst::Tee>     m_out_tee;
};
