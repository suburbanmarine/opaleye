/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/queue.h>

#include <gst/rtsp-server/rtsp-server.h>

#include <glibmm/main.h>

class rtsp_svr_pipe : public GST_element_base
{
public:
  rtsp_svr_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  bool init(const char name[]) override;

  void start(const Glib::RefPtr<Glib::MainContext>& ctx);

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }

protected:

  Glib::RefPtr<Glib::MainContext> m_mainloop_context;

  Glib::RefPtr<Gst::Bin>   m_bin;

  Glib::RefPtr<Gst::Queue> m_in_queue;
  GstRTSPServer*           m_server;
  GstRTSPMountPoints*      m_mounts;
  GstRTSPMediaFactory*     m_factory;
};
