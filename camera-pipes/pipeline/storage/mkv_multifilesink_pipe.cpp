#include "mkv_multifilesink_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

mkv_multifilesink_pipe::mkv_multifilesink_pipe()
{
  
}

void mkv_multifilesink_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool mkv_multifilesink_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool mkv_multifilesink_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}

bool mkv_multifilesink_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue    = Gst::Queue::create();
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 10 * GST_SECOND;

    m_matroskamux = Gst::ElementFactory::create_element("matroskamux");
    // m_matroskamux->set_property("writing-app", "SM");
    m_matroskamux->set_property("version", 2);
    m_matroskamux->set_property("min-index-interval", 100 * 1000*1000);
    // m_matroskamux->set_property("streamable", false);
    m_matroskamux->set_property("streamable", true);
    // m_matroskamux->set_property("timecodescale", );
    // m_matroskamux->set_property("max-cluster-duration", );
    // m_matroskamux->set_property("min-cluster-duration", );

    m_multifilesink = Gst::ElementFactory::create_element("multifilesink");
    // m_multifilesink->set_property("aggregate-gops");
    m_multifilesink->set_property("location", Glib::ustring("file-%06d.mkv"));

    // buffer (0)
    // discont (1)
    // key-frame (2)
    // key-unit-event (3)
    // max-size (4)
    // max-duration (5)
    m_multifilesink->set_property("next-file", 4);
    
    // m_multifilesink->set_property("max-file-duration", );
    // m_multifilesink->set_property("max-file-size", 100*1024*1024);
    // m_multifilesink->set_property("max-files", 1);

    m_bin->add(m_in_queue);
    m_bin->add(m_matroskamux);
    m_bin->add(m_multifilesink);

    m_in_queue->link(m_matroskamux);
    m_matroskamux->link(m_multifilesink);
  }

  return true;
}
