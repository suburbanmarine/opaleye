#include "mkv_multifilesink_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

mkv_multifilesink_pipe::mkv_multifilesink_pipe()/* : m_got_eos(false)*/
{
  
}

void mkv_multifilesink_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
    bin->add(m_bin);
}
void mkv_multifilesink_pipe::remove_from_bin(const Glib::RefPtr<Gst::Bin>& bin)
// {
//     bin->remove(m_bin);
// }

bool mkv_multifilesink_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool mkv_multifilesink_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}

bool mkv_multifilesink_pipe::unlink_front(const Glib::RefPtr<Gst::Element>& node)
{
    node->unlink(m_in_queue);
    return true;
}

bool mkv_multifilesink_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    if( ! m_in_pipe.init("src") )
    {
        return false;
    }

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

    // m_matroskamux->add_probe(GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM, sigc::mem_fun(&mkv_multifilesink_pipe::on_pad_probe, this))
  }

  return true;
}

// void mkv_multifilesink_pipe::send_eos()
// {
//   Glib::RefPtr<Gst::EventEos> eos = Gst::EventEos::create();
//   m_in_queue->send_event(eos);
// }

// void mkv_multifilesink_pipe::install_wait_for_eos()
// {
//     m_got_eos = false;
//     m_eos_probe = m_matroskamux->get_static_pad("sink")->add_probe(Gst::PAD_PROBE_TYPE_BLOCK | Gst::PAD_PROBE_TYPE_EVENT_DOWNSTREAM, sigc::mem_fun(*this, &mkv_multifilesink_pipe::on_pad_probe));
// }
// void mkv_multifilesink_pipe::wait_for_eos()
// {
//     std::unique_lock<std::mutex> lock(m_mutex_got_eos);
//     m_cv_got_eos.wait(lock, [this]{return m_got_eos.load();});
// }
// void mkv_multifilesink_pipe::remove_wait_for_eos()
// {
//     if(m_eos_probe)
//     {
//         m_matroskamux->get_static_pad("sink")->remove_probe(m_eos_probe);
//     }
// }

// Gst::PadProbeReturn mkv_multifilesink_pipe::on_pad_probe(const Glib::RefPtr<Gst::Pad>& pad, const Gst::PadProbeInfo& probe_info)
// {
//     Glib::RefPtr<Gst::Event> event = probe_info.get_event();
//     if(!event)
//     {
//         return Gst::PAD_PROBE_OK;
//     }

//     Gst::PadProbeReturn ret = Gst::PAD_PROBE_OK;

//     switch(event->get_event_type())
//     {
//         case GST_EVENT_EOS:
//         {
//             remove_wait_for_eos();
  
//             unlink_front(m_h264->back());

//             m_got_eos = true;
//             ret = Gst::PAD_PROBE_DROP;
//             break;
//         }
//         default:
//         {
//             ret = Gst::PAD_PROBE_OK;
//             break;
//         }
//     }

//     return ret;
// }