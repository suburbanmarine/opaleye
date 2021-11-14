/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "GST_fakesink.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

GST_fakesink::GST_fakesink()
{
  
}

void GST_fakesink::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool GST_fakesink::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_fakesink);
  return false;
}

bool GST_fakesink::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_fakesink = Gst::FakeSink::create();

    m_bin->add(m_fakesink);
  }

  return true;
}
