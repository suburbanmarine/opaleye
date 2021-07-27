#include "udpsink_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

udpsink_pipe::udpsink_pipe()
{
  
}

void udpsink_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool udpsink_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool udpsink_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}

bool udpsink_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue    = Gst::Queue::create();
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 2 * GST_SECOND;

    m_udpsink = Gst::ElementFactory::create_element("udpsink");
    // m_udpsink->set_property("host", Glib::ustring("127.0.0.1"));
    m_udpsink->set_property("host", Glib::ustring("192.168.21.20"));
    m_udpsink->set_property("port", 50000);


    m_udpsink->set_property("buffer-size",  10 * 1400);
    // m_udpsink->set_property("blocksize",    2 * 1400);
    m_udpsink->set_property("max-lateness", 500 * GST_MSECOND);
    m_udpsink->set_property("processing-deadline", 500 * GST_MSECOND);

    m_bin->add(m_in_queue);
    m_bin->add(m_udpsink);

    m_in_queue->link(m_udpsink);
  }

  return true;
}
