#include "multiudpsink_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

const Glib::SignalProxyInfo multiudpsink_pipe::client_added_signal_info = 
{
  "client-added",
  G_CALLBACK(&Glib::SignalProxyNormal::slot0_void_callback),
  G_CALLBACK(&Glib::SignalProxyNormal::slot0_void_callback)
};
const Glib::SignalProxyInfo multiudpsink_pipe::client_removed_signal_info = 
{
  "client-removed",
  G_CALLBACK(&Glib::SignalProxyNormal::slot0_void_callback),
  G_CALLBACK(&Glib::SignalProxyNormal::slot0_void_callback)
};

multiudpsink_pipe::multiudpsink_pipe()
{
  
}

multiudpsink_pipe::~multiudpsink_pipe()
{
  for(auto sigc : signal_handlers)
  {
      sigc.disconnect();
  }
  signal_handlers.clear();
}

void multiudpsink_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool multiudpsink_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->get_static_pad("src")->link(m_bin_sink);
  return false;
}
bool multiudpsink_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool multiudpsink_pipe::link_front(const Glib::RefPtr<Gst::Pad>& pad)
{
  pad->link(m_bin_sink);
  return true;
}

bool multiudpsink_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue = Gst::Queue::create();
    m_in_queue->property_max_size_buffers() = 0;
    m_in_queue->property_max_size_bytes()   = 0;
    m_in_queue->property_max_size_time()    = 1 * GST_SECOND;

    m_multiudpsink = Gst::ElementFactory::create_element("multiudpsink");
    m_multiudpsink_obj = m_multiudpsink;

    // Glib::SignalProxy<void(GstElement* gstmultiudpsink, gchararray host, gint port, gpointer udata)>(m_multiudpsink->gobj(), &info);


    // Glib::SignalProxyNormal<void(GstElement* gstmultiudpsink, gchararray host, gint port, gpointer udata)>(m_multiudpsink->gobj(), &info).connect(sigc::mem_fun(this, &multiudpsink_pipe::dispatch_client_removed));

    // {
    //   sigc::connection conn = signal_client_added().connect(sigc::mem_fun(*this, &multiudpsink_pipe::handle_client_added));
    //   signal_handlers.push_back(conn);
    // }

    // {
    //   sigc::connection conn = signal_client_removed().connect(sigc::mem_fun(*this, &multiudpsink_pipe::handle_client_removed));
    //   signal_handlers.push_back(conn);
    // }

    // g_signal_connect(m_multiudpsink->gobj(), "client-added",   G_CALLBACK(&multiudpsink_pipe::dispatch_client_added),   this);
    // g_signal_connect(m_multiudpsink->gobj(), "client-removed", G_CALLBACK(&multiudpsink_pipe::dispatch_client_removed), this);

    m_multiudpsink->set_property("buffer-size",  10 * 1400);
    // m_multiudpsink->set_property("blocksize",    2 * 1400);
    m_multiudpsink->set_property("max-lateness", 250 * GST_MSECOND);
    m_multiudpsink->set_property("processing-deadline", 250 * GST_MSECOND);

    // m_multiudpsink->set_property("multicast-iface", "");
    // m_multiudpsink->set_property("auto-multicast", "");

    // m_multiudpsink->set_property("ttl"   , 64);
    // m_multiudpsink->set_property("ttl-mc", 1);

    // m_multiudpsink->set_property("qos-dscp ", -1);

    // m_multiudpsink->set_property("host", Glib::ustring("192.168.21.20"));
    // m_multiudpsink->set_property("port", 50000);    


    m_bin->add(m_in_queue);
    m_bin->add(m_multiudpsink);

    m_in_queue->link(m_multiudpsink);

    m_bin_sink = m_bin->add_ghost_pad(m_in_queue, "sink", "sink");
  }

  return true;
}

void multiudpsink_pipe::handle_client_added(gchararray host, gint port)
{
  const ConnType conn = std::make_pair(host, port);
  {
    std::unique_lock<std::mutex> lock(m_active_conn_mutex);
    m_active_conn.insert(conn);
  }
  m_active_conn_cv.notify_all();
}
void multiudpsink_pipe::handle_client_removed(gchararray host, gint port)
{
  const ConnType conn = std::make_pair(host, port);
  {
    std::unique_lock<std::mutex> lock(m_active_conn_mutex);
    m_active_conn.erase(conn);
  }
  m_active_conn_cv.notify_all();
}

bool multiudpsink_pipe::add_client(const std::string& host, uint16_t port)
{
  g_signal_emit_by_name(m_multiudpsink->gobj(), "add", host.c_str(), port);
  
  const ConnType conn = std::make_pair(host, port);
  bool ret;
  {
    std::unique_lock<std::mutex> lock(m_active_conn_mutex);
    ret = m_active_conn_cv.wait_for(lock, std::chrono::milliseconds(5000), [this,conn]{return has_connection_no_lock(conn);});
  }

  return true;
}
bool multiudpsink_pipe::remove_client(const std::string& host, uint16_t port)
{
  g_signal_emit_by_name(m_multiudpsink->gobj(), "remove", host.c_str(), port);

  const ConnType conn = std::make_pair(host, port);
  bool ret;
  {
    std::unique_lock<std::mutex> lock(m_active_conn_mutex);
    ret = m_active_conn_cv.wait_for(lock, std::chrono::milliseconds(5000), [this,conn]{return ! has_connection_no_lock(conn);});
  }

  return ret;
}

bool multiudpsink_pipe::remove_all_clients(const std::string& host, uint16_t port)
{
  g_signal_emit_by_name(m_multiudpsink->gobj(), "clear");

  const ConnType conn = std::make_pair(host, port);
  bool ret;
  {
    std::unique_lock<std::mutex> lock(m_active_conn_mutex);
    ret = m_active_conn_cv.wait_for(lock, std::chrono::milliseconds(5000), [this,conn]{return m_active_conn.empty();});
  }

  return ret;
}