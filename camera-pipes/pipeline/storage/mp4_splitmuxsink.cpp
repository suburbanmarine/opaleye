#include "mp4_splitmuxsink.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

mp4_splitmuxsink::mp4_splitmuxsink()/* : m_got_eos(false)*/
{
    top_storage_dir = "/opt/suburbanmarine/opaleye/video";
    starting_id     = 0; 
}

void mp4_splitmuxsink::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
    bin->add(m_bin);
}

bool mp4_splitmuxsink::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool mp4_splitmuxsink::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}

bool mp4_splitmuxsink::unlink_front(const Glib::RefPtr<Gst::Element>& node)
{
    node->unlink(m_in_queue);
    return true;
}

bool mp4_splitmuxsink::init(const char name[])
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
    m_splitmuxsink->set_property("location", top_storage_dir.c_str());
    m_splitmuxsink->set_property("start-index", 0);
    m_splitmuxsink->set_property("max-files", 0);
    m_splitmuxsink->set_property("max-size-bytes", 0);
    m_splitmuxsink->set_property("max-size-time",  10*60*GST_SECOND);
    m_splitmuxsink->set_property("send-keyframe-requests",  true); // max-size-bytes must be 0

    g_signal_connect(m_splitmuxsink->gobj(), "format-location", G_CALLBACK(&mp4_splitmuxsink::dispatch_format_location), this);

    // 1.15.1+
    // m_splitmuxsink->set_property("muxer-factory", Glib::ustring("mp4mux"));
    // m_splitmuxsink->set_property("muxer-properties", 
    //     Glib::ustring(
    //         ""
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

gchararray mp4_splitmuxsink::dispatch_format_location(GstElement* splitmux, guint fragment_id, void* ctx)
{
    mp4_splitmuxsink* const inst = static_cast<mp4_splitmuxsink*>(ctx);
    return inst->handle_format_location(splitmux, fragment_id);
}

gchararray mp4_splitmuxsink::handle_format_location(GstElement* splitmux, guint fragment_id)
{
    guint current_fragment_id = fragment_id + starting_id;
    current_filename = fmt::format("file-{:06u}.mp4", current_fragment_id);

    current_path = top_storage_dir / current_filename;

    return g_strdup_printf("%s", 
            current_path.c_str()
        );
}