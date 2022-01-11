/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "timeoverlay.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

timeoverlay::timeoverlay()
{
  
}

void timeoverlay::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool timeoverlay::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_timeoverlay);
  return true;
}
bool timeoverlay::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool timecodestamper::init(const char name[], const Glib::RefPtr<Gst::Bin>& bin)
{
  if(! bin )
  {
    return false;
  }

  m_bin = bin;

  return init(name);
}

bool timeoverlay::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_timeoverlay = Gst::ElementFactory::create_element("timeoverlay", name);
    
    // buffer-time (0) – buffer-time
    // stream-time (1) – stream-time
    // running-time (2) – running-time
    // time-code (3) – time-code
    // elapsed-running-time (4) – elapsed-running-time
    m_timeoverlay->set_property("time-mode ", 5);

    m_timeoverlay->set_property("show-times-as-dates ", false);

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_timeoverlay);
    m_bin->add(m_out_tee);

    m_timeoverlay->link(m_out_tee);
  }

  return true;
}
