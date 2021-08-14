/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

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
