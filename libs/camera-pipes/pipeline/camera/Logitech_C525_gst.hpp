/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "cameras/Logitech_C525.hpp"

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/appsrc.h>

#include <spdlog/spdlog.h>

class Logitech_C525_gst : public GST_element_base
{
public:
  Logitech_C525_gst()
  {
    m_src = Gst::AppSrc::create("C525");

    m_src_caps = Gst::Caps::create_simple(
      "video/x-jpeg",
      "format", Gst::Fourcc('M', 'J', 'P', 'G'),
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "framerate", Gst::Fraction(30, 1),
      "width",  640,
      "height", 480
      );

    m_src->property_caps().set_value(m_src_caps);

    m_src->property_is_live().set_value(true);
    m_src->property_do_timestamp().set_value(true);
    m_src->property_num_buffers().set_value(1);
  }

  bool init(const char name[]) override
  {
    if( ! m_camera.open() )
    {
     SPDLOG_ERROR("Could not open camera");
     return false;
    }

    if( ! m_camera.start() )
    {
     SPDLOG_ERROR("Could not start camera");
     return false;
    }

    return true;
  }

protected:
  Glib::RefPtr<Gst::AppSrc> m_src;
  Glib::RefPtr<Gst::Caps>   m_src_caps;

  Logitech_C525 m_camera;

  uvc_frame_t frame_buffer;
};
