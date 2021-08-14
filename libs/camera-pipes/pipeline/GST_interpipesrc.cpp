/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "GST_interpipesrc.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

GST_interpipesrc::GST_interpipesrc()
{
  
}

void GST_interpipesrc::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool GST_interpipesrc::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool GST_interpipesrc::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool GST_interpipesrc::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_interpipesrc = Gst::ElementFactory::create_element("interpipesrc", name);
    g_object_set(m_interpipesrc->gobj(), "format",      3, NULL); // TIME
    g_object_set(m_interpipesrc->gobj(), "stream-type", 0, NULL); // stream
    g_object_set(m_interpipesrc->gobj(), "stream-sync", 0, NULL); // restart-ts
    g_object_set(m_interpipesrc->gobj(), "is-live",     true, NULL); // restart-ts

    m_out_tee = Gst::Tee::create();

    // m_bin->add(m_in_queue);
    m_bin->add(m_interpipesrc);
    m_bin->add(m_out_tee);

    m_interpipesrc->link(m_out_tee);
  }

  return true;
}

void GST_interpipesrc::set_listen_to(const char name[])
{
  // m_interpipesrc->property("listen-to", name);
  g_object_set(m_interpipesrc->gobj(), "listen-to", name, NULL);
}
