/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>

#include <atomic>
#include <mutex>
#include <condition_variable>

class framegrab_bin : public GST_element_base
{
public:
  framegrab_bin();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;

  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  bool init(const char name[]) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }

  Glib::RefPtr<Gst::Bin> get_bin()
  {
    return m_bin;
  }  

  void set_location(const std::string& s);

protected:

  Glib::RefPtr<Gst::Bin>        m_bin;

  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Element>    m_videorate;
  Glib::RefPtr<Gst::Caps>       m_out_caps;
  Glib::RefPtr<Gst::CapsFilter> m_out_capsfilter;
  Glib::RefPtr<Gst::Element>    m_multifilesink;

  std::string location;
  int index;
};
