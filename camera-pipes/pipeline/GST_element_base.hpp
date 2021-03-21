#pragma once

#include <gstreamermm/element.h>
#include <gstreamermm/bin.h>

#include "pipeline/GST_pipeline_base.hpp"

class GST_element_base
{
public:

	GST_element_base()
	{

	}

	virtual ~GST_element_base()
	{

	}

	virtual void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) = 0;
  virtual void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin, const Glib::RefPtr<Gst::Bus>& bus)
  {
    add_to_bin(bin);
  }
	virtual bool link_front(const Glib::RefPtr<Gst::Element>& node) = 0;
	virtual bool link_back(const Glib::RefPtr<Gst::Element>& node) = 0;

	virtual bool init(const char name[]) = 0;

  virtual Glib::RefPtr<Gst::Element> front()
  {
    return Glib::RefPtr<Gst::Element>();
  }
  virtual Glib::RefPtr<Gst::Element> back()
  {
    return Glib::RefPtr<Gst::Element>();
  }

// gst_element_link_pads
// gst_element_link_pads_filtered
// gst_element_link_pads_full


	// Glib::RefPtr<Gst::Pad> get_static_pad(const char name[])
	// {
	// 	return m_element->get_static_pad(name);
	// }

	// Glib::RefPtr<Gst::Element> get_element()
	// {
	// 	return m_element;
	// }

protected:

	
};
