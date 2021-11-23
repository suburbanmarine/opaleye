/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/fakesink.h>

class GST_fakesink : public GST_element_base
{
public:
  GST_fakesink();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_fakesink;
  }
  Glib::RefPtr<Gst::Element> back() override
  {
    return m_fakesink;
  }

  bool init(const char name[]) override;

protected:

  Glib::RefPtr<Gst::Bin>           m_bin;

  Glib::RefPtr<Gst::FakeSink> m_fakesink;
};
