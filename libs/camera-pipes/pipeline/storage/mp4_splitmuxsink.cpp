#include "mp4_splitmuxsink.hpp"

#include <gstreamermm/elementfactory.h>

#include "boost/date_time/posix_time/posix_time_types.hpp"

#include <boost/range/iterator_range.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/bundled/printf.h>

mp4_splitmuxsink::mp4_splitmuxsink()/* : m_got_eos(false)*/
{
    // top_storage_dir = "/opt/suburbanmarine/opaleye/video";
    top_storage_dir = "/home/rounin/suburbanmarine/opaleye";
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
    //get todays folder
    {
        // boost::date_time::time_point t0;
        boost::posix_time::ptime t0 = boost::posix_time::microsec_clock::universal_time();
        std::string date_str = fmt::format("{:04d}{:02d}{:02d}", t0.date().year(), t0.date().month(), t0.date().day());
        top_storage_dir = top_storage_dir / date_str;
    }

    SPDLOG_INFO("mp4_splitmuxsink::init Checking dir {:s}", top_storage_dir.string());

    //scan for index
    boost::filesystem::file_status top_storage_dir_status = boost::filesystem::status(top_storage_dir);
    if(boost::filesystem::is_directory(top_storage_dir_status))
    {
        std::string filename;
        for(auto& dir_entry : boost::make_iterator_range(boost::filesystem::directory_iterator(top_storage_dir), {}))
        {
            boost::filesystem::file_status dir_entry_status = boost::filesystem::status(dir_entry);
            if(boost::filesystem::is_regular_file(dir_entry_status))
            {
                unsigned int num = 0;
                filename = dir_entry.path().filename().string();
                int ret = sscanf(filename.c_str(), "file-%06u.mp4", &num);
                if(ret == 1)
                {
                    starting_id = std::max<guint>(starting_id, num + 1);
                }
            }
        }

        SPDLOG_INFO("mp4_splitmuxsink::init Starting from index {:d}", starting_id);
    }
    else
    {
        SPDLOG_INFO("mp4_splitmuxsink::init Dir {:s} does not exist, creating", top_storage_dir.string());
        boost::filesystem::create_directory(top_storage_dir);
    }

    //init our internal bin and elements
    {
      m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());
  
      m_in_queue    = Gst::Queue::create();
      m_in_queue->property_max_size_buffers()      = 0;
      m_in_queue->property_max_size_bytes()        = 0;
      m_in_queue->property_max_size_time()         = 10 * GST_SECOND;
  
      m_splitmuxsink = Gst::ElementFactory::create_element("splitmuxsink");
      // m_splitmuxsink->set_property("async-finalize", true); // 1.15.1+
      // m_splitmuxsink->set_property("location", top_storage_dir.c_str());
      // m_splitmuxsink->set_property("start-index", 0);
      m_splitmuxsink->set_property("max-files", 0);
      m_splitmuxsink->set_property("max-size-bytes", 0);
      m_splitmuxsink->set_property("max-size-time",  10*60*GST_SECOND);
      // m_splitmuxsink->set_property("send-keyframe-requests",  true); // max-size-bytes must be 0
      m_splitmuxsink->set_property<Glib::ustring>("muxer", "avimux");

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
    current_filename = fmt::format("file-{:06d}.avi", current_fragment_id);

    current_path = top_storage_dir / current_filename;

    return g_strdup_printf("%s", current_path.c_str());
}