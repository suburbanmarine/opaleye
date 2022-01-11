/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "timecodestamper.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

timecodestamper::timecodestamper()
{
  
}

void timecodestamper::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool timecodestamper::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_nvvidconv);
  return true;
}
bool timecodestamper::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool timecodestamper::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_timecodestamper = Gst::ElementFactory::create_element("timecodestamper", name);
    
    // internal (0) – Use internal timecode counter, starting at zero or value set by property
    // zero (1) – Always use zero
    // last-known (2) – Count up from the last known upstream timecode or internal if unknown
    // last-known-or-zero (3) – Count up from the last known upstream timecode or zero if unknown
    // ltc (4) – Linear timecode from an audio device
    // rtc (5) – Timecode from real time clock
    m_timecodestamper->set_property("source ", 5);

    // never (0) – Never set timecodes
    // keep (1) – Keep upstream timecodes and only set if no upstream timecode
    // always (2) – Always set timecode and remove upstream timecode
    m_timecodestamper->set_property("set ", 2);

    m_timecodestamper->set_property("rtc-auto-resync", true);

    m_timecodestamper->set_property("post-messages", true);

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_timecodestamper);
    m_bin->add(m_out_tee);

    m_timecodestamper->link(m_out_tee);
  }

  return true;
}
