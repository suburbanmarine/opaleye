/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "Testsrc_pipe.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

Testsrc_pipe::Testsrc_pipe()
{
  
}

void Testsrc_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}

bool Testsrc_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool Testsrc_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool Testsrc_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    //source
    m_src = Gst::VideoTestSrc::create();
    // frames (0) – frame count
    // wall-time (1) – wall clock time
    // running-time (2) – running time
    m_src->set_property("animation_mode", 1);
    m_src->property_is_live()        = true;
    // wavy (0) – Ball waves back and forth, up and down
    // sweep (1) – 1 revolution per second
    // hsweep (2) – 1/2 revolution per second, then reset to top
    m_src->set_property("motion", 0);
    // VIDEO_TEST_SRC_SMPTE,
    // VIDEO_TEST_SRC_SNOW,
    // VIDEO_TEST_SRC_BLACK,
    // VIDEO_TEST_SRC_WHITE,
    // VIDEO_TEST_SRC_RED,
    // VIDEO_TEST_SRC_GREEN,
    // VIDEO_TEST_SRC_BLUE,
    // VIDEO_TEST_SRC_CHECKERS1,
    // VIDEO_TEST_SRC_CHECKERS2,
    // VIDEO_TEST_SRC_CHECKERS4,
    // VIDEO_TEST_SRC_CHECKERS8,
    // VIDEO_TEST_SRC_CIRCULAR,
    // VIDEO_TEST_SRC_BLINK,
    // VIDEO_TEST_SRC_SMPTE75,
    // VIDEO_TEST_SRC_ZONE_PLATE,
    // VIDEO_TEST_SRC_GAMUT,
    // VIDEO_TEST_SRC_CHROMA_ZONE_PLATE,
    // VIDEO_TEST_SRC_SOLID,
    // VIDEO_TEST_SRC_BALL,
    // VIDEO_TEST_SRC_SMPTE100,
    // VIDEO_TEST_SRC_BAR,
    // VIDEO_TEST_SRC_PINWHEEL,
    // VIDEO_TEST_SRC_SPOKES,
    // VIDEO_TEST_SRC_GRADIENT,
    // VIDEO_TEST_SRC_COLORS
    m_src->property_pattern()        = Gst::VIDEO_TEST_SRC_ZONE_PLATE;

    //for zone plate
    m_src->set_property("kx2", 20);
    m_src->set_property("ky2", 20);
    m_src->set_property("kt", 1);

    //src caps
    m_src_caps = Gst::Caps::create_simple(
      "video/x-raw",
      // "format", Gst::Fourcc('M', 'J', 'P', 'G'),
      // "format", Gst::Fourcc(Glib::ustring("MJPG")),
      // "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "format", "I420",
      "framerate", Gst::Fraction(30, 1),
      "width",  640,
      "height", 480
      );

    m_src_capsfilter = Gst::CapsFilter::create();
    m_src_capsfilter->property_caps().set_value(m_src_caps);

    m_in_queue     = Gst::Queue::create();
    // m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    // m_in_queue->property_min_threshold_time()    = 0;
    // m_in_queue->property_min_threshold_buffers() = 0;
    // m_in_queue->property_min_threshold_bytes()   = 0;
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 1 * GST_SECOND;

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_src);
    m_bin->add(m_src_capsfilter);
    m_bin->add(m_in_queue);
    m_bin->add(m_out_tee);
  }

  m_src->link(m_src_capsfilter);
  m_src_capsfilter->link(m_in_queue);
  m_in_queue->link(m_out_tee);
  
  return true;
}

