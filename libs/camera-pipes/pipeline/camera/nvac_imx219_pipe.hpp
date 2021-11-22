/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

// https://developer.ridgerun.com/wiki/index.php?title=Xavier/Video_Capture_and_Display/Cameras/IMX219#Gstreamer_examples

#pragma once

#include "v4l2_util.hpp"

#include "pipeline/GST_element_base.hpp"

#include "errno_util.hpp"

#include <gstreamermm/caps.h>
#include <gstreamermm/capsfilter.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/tee.h>

#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <set>

class nvac_imx219_pipe : public GST_element_base
{
public:
  nvac_imx219_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> back() override
  {
    return m_out_tee;
  }

  void set_sensor_id(int val);

  bool init(const char name[]) override;

  bool set_exposure_mode(int32_t val);
  bool get_exposure_mode(int32_t* const val);

  bool set_exposure_value(int32_t val);
  bool get_exposure_value(int32_t* const val);

  bool set_focus_absolute(int32_t val);
  bool get_focus_absolute(int32_t* const val);

  bool set_focus_auto(bool val);
  bool get_focus_auto(bool* const val);

  bool set_brightness(int32_t val);
  bool get_brightness(int32_t* const val);

  bool set_gain(int32_t val);
  bool get_gain(int32_t* const val);
  
protected:

  // bool on_bus_message(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message);
  void on_bus_message(const Glib::RefPtr<Gst::Message>& message);
  // void on_pad_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

  Glib::RefPtr<Gst::Bin>        m_bin;
  Glib::RefPtr<Gst::Bus>        m_bus;

  Glib::RefPtr<Gst::Element>    m_src;
  Glib::RefPtr<Gst::Caps>       m_src_caps;
  Glib::RefPtr<Gst::CapsFilter> m_in_capsfilter;
  Glib::RefPtr<Gst::Element>    m_videorate;
  Glib::RefPtr<Gst::Caps>       m_out_caps;
  Glib::RefPtr<Gst::CapsFilter> m_out_capsfilter;
  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Tee>        m_out_tee;
};