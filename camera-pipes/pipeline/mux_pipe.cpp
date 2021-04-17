#include "mux_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

mux_pipe::mux_pipe()
{
  
}

void mux_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool mux_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  Glib::RefPtr<Gst::Pad> node_src_pad = node->get_static_pad("src");

  Glib::ustring node_name         = node->get_name();
  Glib::ustring node_src_pad_name = node_src_pad->get_name();

  std::string new_pad_name = node_name.raw();
  new_pad_name            += node_src_pad_name.raw();

  m_in_pads[new_pad_name] = node_src_pad;

  node->link(m_inputselector);

  return true;
}
bool mux_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool mux_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    // m_in_queue = Gst::Queue::create();
    // m_in_queue->property_max_size_buffers()      = 0;
    // m_in_queue->property_max_size_bytes()        = 0;
    // m_in_queue->property_max_size_time()         = 1 * GST_SECOND;

    m_inputselector = Gst::InputSelector::create();
    m_inputselector->property_sync_mode()     = Gst::INPUT_SELECTOR_SYNC_MODE_CLOCK;
    m_inputselector->property_sync_streams()  = true;
    m_inputselector->property_cache_buffers() = true;

    //output tee
    m_out_tee = Gst::Tee::create();

    // m_bin->add(m_in_queue);
    m_bin->add(m_inputselector);
    m_bin->add(m_out_tee);

    m_inputselector->link(m_out_tee);
  }

  return true;
}


bool mux_pipe::set_active_pad(const Glib::RefPtr<Gst::Pad>& pad)
{
  m_inputselector->property_active_pad() = pad;

  return get_active_pad() == pad;
}
bool mux_pipe::set_active_pad(const std::string& name)
{
  auto it = m_in_pads.find(name);
  if(it == m_in_pads.end())
  {
    return false;
  }

  m_inputselector->property_active_pad() = it->second;

  return true;
}
Glib::RefPtr<Gst::Pad> mux_pipe::get_active_pad()
{
  return m_inputselector->property_active_pad();
}