/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "V4L2_alvium_pipe.hpp"
#include "v4l2_util.hpp"

#include "pipeline/gst_common.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <sys/ioctl.h>

// v4l2-ctl --list-devices
// ALVIUM 1800 C-507c 9-3c (platform:15c10000.vi:0):
//     /dev/video0
//
// ALVIUM 1800 C-507c 10-3c (platform:15c10000.vi:2):
//     /dev/video1


// v4l2-ctl -d /dev/video0 --list-formats-ext
// ioctl: VIDIOC_ENUM_FMT
//     Index       : 0
//     Type        : Video Capture
//     Pixel Format: 'TP31'
//     Name        : 0x31 MIPI DATATYPE
//         Size: Discrete 2464x2056
//             Interval: Discrete 0.060s (16.593 fps)
//
//     Index       : 1
//     Type        : Video Capture
//     Pixel Format: 'GREY'
//     Name        : 8-bit Greyscale
//         Size: Discrete 2464x2056
//             Interval: Discrete 0.060s (16.593 fps)
//
//     Index       : 2
//     Type        : Video Capture
//     Pixel Format: 'RGGB'
//     Name        : 8-bit Bayer RGRG/GBGB
//         Size: Discrete 2464x2056
//             Interval: Discrete 0.060s (16.593 fps)
//
//     Index       : 3
//     Type        : Video Capture
//     Pixel Format: 'JXY0'
//     Name        : 10-bit/16-bit Greyscale
//         Size: Discrete 2464x2056
//             Interval: Discrete 0.060s (16.593 fps)
//
//     Index       : 4
//     Type        : Video Capture
//     Pixel Format: 'JXR0'
//     Name        : 10-bit/16-bit Bayer RGRG/GBGB
//         Size: Discrete 2464x2056
//             Interval: Discrete 0.060s (16.593 fps)
//
//     Index       : 5
//     Type        : Video Capture
//     Pixel Format: 'JXY2'
//     Name        : 12-bit/16-bit Greyscale
//         Size: Discrete 2464x2056
//             Interval: Discrete 0.060s (16.593 fps)
//
//     Index       : 6
//     Type        : Video Capture
//     Pixel Format: 'JXR2'
//     Name        : 12-bit/16-bit Bayer RGRG/GBGB
//         Size: Discrete 2464x2056
//             Interval: Discrete 0.060s (16.593 fps)
//
//     Index       : 7
//     Type        : Video Capture
//     Pixel Format: 'BX24'
//     Name        : 32-bit XRGB 8-8-8-8
//         Size: Discrete 2464x2056
//             Interval: Discrete 0.060s (16.593 fps)
//
//     Index       : 8
//     Type        : Video Capture
//     Pixel Format: 'XR24'
//     Name        : 32-bit BGRX 8-8-8-8
//         Size: Discrete 2464x2056
//             Interval: Discrete 0.060s (16.593 fps)
//
//     Index       : 9
//     Type        : Video Capture
//     Pixel Format: 'VYUY'
//     Name        : VYUY 4:2:2
//         Size: Discrete 2464x2056
//             Interval: Discrete 0.060s (16.593 fps)



V4L2_alvium_pipe::V4L2_alvium_pipe()
{
  
}

void V4L2_alvium_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);

  m_bus = bin->get_bus();
}

bool V4L2_alvium_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool V4L2_alvium_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  try
  {
    m_out_tee->link(node);
    return true;
  }
  catch(const std::exception& e)
  {
    SPDLOG_ERROR("Failed to link back: {:s}", e.what());
  }
  catch(...)
  {
    SPDLOG_ERROR("Failed to link back, unknown exception"); 
  }

  return false;
}

bool V4L2_alvium_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());
    if(! m_bin )
    {
      SPDLOG_ERROR("Failed to create bin");
      return false;
    }

    //source
    m_src = Gst::ElementFactory::create_element("v4l2src", name);
    if(! m_src )
    {
      SPDLOG_ERROR("Failed to create src");
      return false;
    }

    m_src->set_property("do-timestamp", true);
    m_src->set_property("is-live", true);
    m_src->set_property("device", Glib::ustring("/dev/video0"));

	// (0): auto             - GST_V4L2_IO_AUTO
	// (1): rw               - GST_V4L2_IO_RW
	// (2): mmap             - GST_V4L2_IO_MMAP
	// (3): userptr          - GST_V4L2_IO_USERPTR
	// (4): dmabuf           - GST_V4L2_IO_DMABUF
	// (5): dmabuf-import    - GST_V4L2_IO_DMABUF_IMPORT
    m_src->set_property("io-mode", 2);
    // m_src->add_probe(GST_PAD_PROBE_TYPE_IDLE | GST_PAD_PROBE_TYPE_EVENT_BOTH, sigc::mem_fun(&V4L2_alvium_pipe::on_pad_probe, this))

    //src caps
    // m_src_caps = Gst::Caps::create_simple(
    //   "video/x-raw",
    //   "format","(string)BGRx",
    //   "pixel-aspect-ratio", Gst::Fraction(1, 1),
    //   "framerate",          Gst::Fraction(16593, 1000),
    //   "width",              2464,
    //   "height",             2056
    // );

    // m_src_caps = Gst::Caps::create_from_string("video/x-raw, width=(int)2464, height=(int)2056, format=(string)BGRx, framerate=(fraction)16593/1000, pixel-aspect-ratio=(fraction)1/1");
    m_src_caps = Gst::Caps::create_from_string("video/x-raw, width=(int)2464, height=(int)2056, format=(string)BGRx, pixel-aspect-ratio=(fraction)1/1");

    // m_src_caps = Gst::Caps::create_simple(
    //   "video/x-bayer",
    //   "format","rggb",
    //   "pixel-aspect-ratio", Gst::Fraction(1, 1),
    //   "framerate",          Gst::Fraction(16593, 1000),
    //   "width",              2464,
    //   "height",             2056
    // );

    if(! m_src_caps )
    {
      SPDLOG_ERROR("Failed to create m_src_caps");
    }

    m_in_capsfilter = Gst::CapsFilter::create("incaps");
    if(! m_in_capsfilter )
    {
      SPDLOG_ERROR("Failed to create m_in_capsfilter");
      return false;
    }

    m_in_capsfilter->property_caps() = m_src_caps;

    m_in_queue     = Gst::Queue::create();
    if(! m_in_queue )
    {
      SPDLOG_ERROR("Failed to create m_in_queue");
      return false;
    }

    // m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    // m_in_queue->property_min_threshold_time()    = 0;
    // m_in_queue->property_min_threshold_buffers() = 0;
    // m_in_queue->property_min_threshold_bytes()   = 0;
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 1 * GST_SECOND;

    //output tee
    m_out_tee = Gst::Tee::create();
    if(! m_out_tee )
    {
      SPDLOG_ERROR("Failed to create m_out_tee");
      return false;
    }

    m_bin->add(m_src);
    m_bin->add(m_in_capsfilter);
    m_bin->add(m_in_queue);
    m_bin->add(m_out_tee);
  }

  m_src->link(m_in_capsfilter);
  m_in_capsfilter->link(m_in_queue);
  m_in_queue->link(m_out_tee);

  return true;
}
