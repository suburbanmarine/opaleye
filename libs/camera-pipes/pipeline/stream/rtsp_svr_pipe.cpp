#include "rtsp_svr_pipe.hpp"

#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

GstElement* create_bin_element(GstRTSPMediaFactory *factory, const GstRTSPUrl *url)
{
  return NULL;
}

void media_configure(GstRTSPMediaFactory * factory, GstRTSPMedia * media, gpointer user_data)
{
  
}

rtsp_svr_pipe::rtsp_svr_pipe()
{
  
}

void rtsp_svr_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool rtsp_svr_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}
bool rtsp_svr_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}

bool rtsp_svr_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_in_queue = Gst::Queue::create();

    m_server   = gst_rtsp_server_new();
    g_object_set (m_server, "service", 60000, NULL);

    m_mounts = gst_rtsp_server_get_mount_points (m_server);

    m_factory = gst_rtsp_media_factory_new();
    gst_rtsp_media_factory_set_launch(m_factory, "");
    g_signal_connect (m_factory, "media-configure", (GCallback) media_configure, NULL);

    gst_rtsp_mount_points_add_factory (m_mounts, "/test", m_factory);


    // m_in_queue->link(m_rtph264pay);
  }

  return true;
}

void rtsp_svr_pipe::start(const Glib::RefPtr<Glib::MainContext>& ctx)
{
  m_mainloop_context = ctx;
  gst_rtsp_server_attach(m_server, m_mainloop_context->gobj());
}