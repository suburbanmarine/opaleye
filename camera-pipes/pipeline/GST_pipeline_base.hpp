#pragma once

#include <gstreamermm/element.h>
#include <gstreamermm/pipeline.h>
#include <gstreamermm/bus.h>

class GST_pipeline_base
{
public:

	GST_pipeline_base(const char name[]);

	virtual ~GST_pipeline_base();

protected:

  Glib::RefPtr<Gst::Element> m_pipe;
  Glib::RefPtr<Gst::Bus>     m_bus;
};
