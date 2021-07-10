#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/bin.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/structure.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

#include <gst/gstcaps.h>
#include <gst/gststructure.h>

#include <gst/sdp/gstsdpmessage.h>

class rtp_mjpeg_pipe : public GST_element_base
{
public:
  rtp_mjpeg_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  bool init(const char name[]) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }

  Glib::RefPtr<Gst::Caps> get_current_caps()
  {
    return m_rtpmjpegpay->get_static_pad("src")->get_current_caps();
  }

  std::string get_current_caps_str()
  {
    Glib::RefPtr<Gst::Caps> caps = get_current_caps();

  /*
   * The current implementation of serialization will lead to unexpected results
   * when there are nested Gst::Caps / Gst::Structure deeper than one level.
   */

    return caps->to_string();
  }

  std::string get_sdp(const char stream_addr[], const char stream_orig[], uint16_t dest_port)
  {
    
    //https://cgit.freedesktop.org/gstreamer/gst-plugins-good/tree/gst/rtp/README

    // https://thiblahute.github.io/GStreamer-doc/gst-plugins-base-sdp-1.0/gstsdpmessage.html?gi-language=c
    // gst_sdp_media_set_media_from_caps
    Glib::RefPtr<Gst::Caps> caps = get_current_caps();

    GstSDPMedia* media_video = NULL;
    gst_sdp_media_new(&media_video);
    GstSDPResult ret = gst_sdp_media_set_media_from_caps(caps->gobj(), media_video);
    gst_sdp_media_add_connection(media_video, 
      "IN",         // nettype
      "IP4",        // addrtype
      stream_orig,  // addr
      64,           // TTL
      1             // Layer count
      );

    gst_sdp_media_set_port_info(media_video, dest_port, 1);

    gst_sdp_media_get_attribute_val(media_video, "rtpmap");
    gst_sdp_media_set_media(media_video, "video");

    GstSDPMessage* message = NULL;
    gst_sdp_message_new(&message);

    gst_sdp_message_set_version(message, "0");

    gst_sdp_message_set_origin(message,
      "-",
      "",
      "",
      "IN",
      "IP4",
      "127.0.0.1"
      );

    gst_sdp_media_set_information(media_video, "Opaleye stream");
    
    gst_sdp_message_add_media(message, media_video);

    gchar* msg_gchar = gst_sdp_message_as_text(message);

    std::string msg_string(msg_gchar);

    gst_sdp_media_free(media_video);
    g_free(msg_gchar);

    return msg_string;
  }

protected:

  Glib::RefPtr<Gst::Bin>        m_bin;

  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Element>    m_rtpmjpegpay; 
  Glib::RefPtr<Gst::Tee>        m_out_tee;
};
