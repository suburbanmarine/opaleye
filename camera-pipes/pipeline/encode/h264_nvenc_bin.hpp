#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

class h264_nvenc_bin : public GST_element_base
{
public:
  h264_nvenc_bin();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }
  Glib::RefPtr<Gst::Element> back() override
  {
    return m_out_tee;
  }

  bool init(const char name[]) override;

protected:

  Glib::RefPtr<Gst::Bin>        m_bin;

  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Element>    m_videoconvert;
  Glib::RefPtr<Gst::Element>    m_omxh264enc;
  Glib::RefPtr<Gst::Element>    m_h264parse;
  Glib::RefPtr<Gst::Caps>       m_out_caps;
  Glib::RefPtr<Gst::CapsFilter> m_capsfilter;
  Glib::RefPtr<Gst::Tee>        m_out_tee;
};