#include "mkv_splitmuxsink.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

mkv_splitmuxsink::mkv_splitmuxsink()/* : m_got_eos(false)*/
{
    location = "file-%06d.mkv";
    index    = 0; 
}

void mkv_splitmuxsink::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
    bin->add(m_bin);
}

bool mkv_splitmuxsink::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool mkv_splitmuxsink::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}

bool mkv_splitmuxsink::unlink_front(const Glib::RefPtr<Gst::Element>& node)
{
    node->unlink(m_in_queue);
    return true;
}

bool mkv_splitmuxsink::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue    = Gst::Queue::create();
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 10 * GST_SECOND;

    m_splitmuxsink = Gst::ElementFactory::create_element("splitmuxsink");
    // m_splitmuxsink->set_property("async-finalize", true); // 1.15.1+
    m_splitmuxsink->set_property("location", location);
    m_splitmuxsink->set_property("start-index", 0);
    m_splitmuxsink->set_property("max-files", 0);
    m_splitmuxsink->set_property("max-size-bytes", 0);
    m_splitmuxsink->set_property("max-size-time",  10*60*GST_SECOND);
    m_splitmuxsink->set_property("send-keyframe-requests",  true); // max-size-bytes must be 0

    // 1.15.1+
    // m_splitmuxsink->set_property("muxer-factory", Glib::ustring("matroskamux"));
    // m_splitmuxsink->set_property("muxer-properties", 
    //     Glib::ustring(
    //         "version=2,"
    //         "streamable=false,"
    //         "writing-app=\"cam-pod\","
    //         "offset-to-zero=true"
    //         // "min-index-interval=500000000"
    //     )
    // );

    // 1.15.1+
    // m_splitmuxsink->set_property("sink-factory", "");
    // m_splitmuxsink->set_property("sink-properties", 
    //     ""
    //     );

    m_bin->add(m_in_queue);
    m_bin->add(m_splitmuxsink);

    m_in_queue->link(m_splitmuxsink);
  }

  return true;
}

void mkv_splitmuxsink::set_location(const std::string& s)
{

}

