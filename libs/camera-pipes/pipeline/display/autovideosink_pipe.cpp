/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "autovideosink_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

autovideosink_pipe::autovideosink_pipe()
{
  
}

void autovideosink_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool autovideosink_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool autovideosink_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}

bool autovideosink_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue    = Gst::Queue::create();
    // m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 1 * GST_SECOND;

    // m_in_queue->property_min_threshold_buffers() = 0;
    // m_in_queue->property_min_threshold_bytes()   = 0;
    // m_in_queue->property_min_threshold_time()    = 2 * GST_SECOND;

    m_videoconvert  = Gst::ElementFactory::create_element("videoconvert");

    m_disp_queue    = Gst::Queue::create();
    // m_disp_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    m_disp_queue->property_max_size_buffers()      = 0;
    m_disp_queue->property_max_size_bytes()        = 0;
    m_disp_queue->property_max_size_time()         = 1 * GST_SECOND;

    // m_autovideosink = Gst::ElementFactory::create_element("autovideosink");
    m_autovideosink = Gst::ElementFactory::create_element("xvimagesink");
    // m_autovideosink->set_property("sync", false);
    // m_autovideosink->set_property("max-lateness", 250 * GST_MSECOND);
    // m_autovideosink->set_property("processing-deadline", 250 * GST_MSECOND);

    m_bin->add(m_in_queue);
    m_bin->add(m_videoconvert);
    m_bin->add(m_disp_queue);
    m_bin->add(m_autovideosink);

    m_in_queue->link(m_videoconvert);
    m_videoconvert->link(m_disp_queue);
    m_disp_queue->link(m_autovideosink);
  }

  return true;
}
