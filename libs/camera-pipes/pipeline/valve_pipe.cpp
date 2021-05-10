#include "valve_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

valve_pipe::valve_pipe()
{
  
}

void valve_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool valve_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_valve);
  return false;
}
bool valve_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool valve_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    // m_in_queue = Gst::Queue::create();
    // m_in_queue->property_max_size_buffers()      = 0;
    // m_in_queue->property_max_size_bytes()        = 0;
    // m_in_queue->property_max_size_time()         = 1 * GST_SECOND;

    m_valve = Gst::Valve::create();

    //output tee
    m_out_tee = Gst::Tee::create();

    // m_bin->add(m_in_queue);
    m_bin->add(m_valve);
    m_bin->add(m_out_tee);

    m_valve->link(m_out_tee);
  }

  return true;
}

void valve_pipe::set_drop(const bool drop)
{
  m_valve->property_drop() = drop;
}