#include "GST_interpipesink.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

GST_interpipesink::GST_interpipesink()
{
  
}

void GST_interpipesink::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool GST_interpipesink::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_interpipesink);
  return true;
}
bool GST_interpipesink::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool GST_interpipesink::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue = Gst::Queue::create();
    m_interpipesink = Gst::ElementFactory::create_element("interpipesink", name);

    // m_bin->add(m_in_queue);
    m_bin->add(m_in_queue);
    m_bin->add(m_interpipesink);

    m_in_queue->link(m_interpipesink);
  }

  return true;
}

// bool GST_interpipesrc::set_listen_to(const char name[])
// {
//   m_interpipesink.property("listen-to", name);
// }