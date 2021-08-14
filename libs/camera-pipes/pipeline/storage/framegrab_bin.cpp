/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "framegrab_bin.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

framegrab_bin::framegrab_bin()/* : m_got_eos(false)*/
{
    location = "file-%08d.jpg";
    index    = 0; 
}

void framegrab_bin::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
    bin->add(m_bin);
}

bool framegrab_bin::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool framegrab_bin::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool framegrab_bin::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue    = Gst::Queue::create();
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 10 * GST_SECOND;

    m_videorate    = Gst::ElementFactory::create_element("videorate");
    
    //caps
    m_out_caps = Gst::Caps::create_simple(
      "video/x-raw",
      // "format", Gst::Fourcc('M', 'J', 'P', 'G'),
      // "format", Gst::Fourcc(Glib::ustring("MJPG")),
      // "pixel-aspect-ratio", Gst::Fraction(1, 1),
      // "format", "I420",
      "framerate", Gst::Fraction(1, 5),
      "width",  640,
      "height", 480
      );

    m_out_capsfilter = Gst::CapsFilter::create();
    m_out_capsfilter->property_caps().set_value(m_out_caps);

    m_multifilesink = Gst::ElementFactory::create_element("multifilesink");
    m_multifilesink->set_property("location", location);
    m_multifilesink->set_property("index", index);
    // m_multifilesink->set_property("aggregate-gops", true);

    // buffer (0) – New file for each buffer
    // discont (1) – New file after each discontinuity
    // key-frame (2) – New file at each key frame (Useful for MPEG-TS segmenting)
    // key-unit-event (3) – New file after a force key unit event
    // max-size (4) – New file when the configured maximum file size would be exceeded with the next buffer or buffer list
    // max-duration (5) – New file when the configured maximum file duration would be exceeded with the next buffer or buffer list
    m_multifilesink->set_property("next-file", 0);
    
    // m_multifilesink->set_property("max-file-duration", );
    // m_multifilesink->set_property("max-file-size", 100*1024*1024);
    // m_multifilesink->set_property("max-files", 1);

    m_bin->add(m_in_queue);
    m_bin->add(m_videorate);
    m_bin->add(m_out_capsfilter);
    m_bin->add(m_multifilesink);

    m_in_queue->link(m_videorate);
    m_videorate->link(m_out_capsfilter);
    m_out_capsfilter->link(m_multifilesink);
  }

  return true;
}
