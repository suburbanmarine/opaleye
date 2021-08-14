/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/inputselector.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

class GST_interpipesink : public GST_element_base
{
public:
  GST_interpipesink();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }

  bool init(const char name[]) override;

protected:

  Glib::RefPtr<Gst::Bin>     m_bin;

  Glib::RefPtr<Gst::Queue>   m_in_queue;
  Glib::RefPtr<Gst::Element> m_interpipesink;
};
