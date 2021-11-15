/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "nvvideoconvert_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

nvvideoconvert_pipe::nvvideoconvert_pipe()
{
  
}

void nvvideoconvert_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool nvvideoconvert_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_nvvidconv);
  return true;
}
bool nvvideoconvert_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool nvvideoconvert_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    // m_in_queue = Gst::Queue::create();
    // m_in_queue->property_max_size_buffers()      = 0;
    // m_in_queue->property_max_size_bytes()        = 0;
    // m_in_queue->property_max_size_time()         = 1 * GST_SECOND;

    m_nvvidconv = Gst::ElementFactory::create_element("nvvideoconvert", name);

    //output tee
    m_out_tee = Gst::Tee::create();

    // m_bin->add(m_in_queue);
    m_bin->add(m_nvvidconv);
    m_bin->add(m_out_tee);

    m_nvvidconv->link(m_out_tee);
  }

  return true;
}
