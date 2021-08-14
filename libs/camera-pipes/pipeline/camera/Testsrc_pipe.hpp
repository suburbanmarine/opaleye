/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "cameras/Logitech_brio.hpp"

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/videotestsrc.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/caps.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

#include <atomic>
#include <memory>
#include <mutex>

class Testsrc_pipe : public GST_element_base
{
public:
  Testsrc_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> back() override
  {
    return m_out_tee;
  }

  bool init(const char name[]) override;

protected:

  bool on_bus_message(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message);

  Glib::RefPtr<Gst::Bin>        m_bin;
  Glib::RefPtr<Gst::Bus>        m_bus;

  Glib::RefPtr<Gst::VideoTestSrc> m_src;
  Glib::RefPtr<Gst::Caps>         m_src_caps;
  Glib::RefPtr<Gst::CapsFilter>   m_src_capsfilter;
  Glib::RefPtr<Gst::Queue>        m_in_queue;
  Glib::RefPtr<Gst::Tee>          m_out_tee;

};
