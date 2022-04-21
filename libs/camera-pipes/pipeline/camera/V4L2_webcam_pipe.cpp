/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "V4L2_webcam_pipe.hpp"
#include "util/v4l2_util.hpp"

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
// Logitech BRIO (usb-0000:03:00.3-1.1.4.1):
//   /dev/video2
//   /dev/video3
//   /dev/video4
//   /dev/video5
// 
// HD Webcam C525 (usb-0000:4c:00.1-3):
//   /dev/video0
//   /dev/video1

// v4l2-ctl -d /dev/video2 --list-formats-ext
// ioctl: VIDIOC_ENUM_FMT
//   Type: Video Capture
//
//   [0]: 'YUYV' (YUYV 4:2:2)
//     Size: Discrete 640x480
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 160x120
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 176x144
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 320x180
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 320x240
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 352x288
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 340x340
//       Interval: Discrete 0.033s (30.000 fps)
//     Size: Discrete 424x240
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 440x440
//       Interval: Discrete 0.033s (30.000 fps)
//     Size: Discrete 480x270
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 640x360
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 800x448
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 800x600
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 848x480
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 960x540
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 1024x576
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 1280x720
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 1600x896
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 1920x1080
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//   [1]: 'MJPG' (Motion-JPEG, compressed)
//     Size: Discrete 640x480
//       Interval: Discrete 0.008s (120.000 fps)
//       Interval: Discrete 0.011s (90.000 fps)
//       Interval: Discrete 0.017s (60.000 fps)
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 160x120
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 176x144
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 320x180
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 320x240
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 352x288
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 424x240
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 480x270
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 640x360
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 800x448
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 800x600
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 848x480
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 960x540
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 1024x576
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 1280x720
//       Interval: Discrete 0.011s (90.000 fps)
//       Interval: Discrete 0.017s (60.000 fps)
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 1600x896
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 1920x1080
//       Interval: Discrete 0.017s (60.000 fps)
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 2560x1440
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 3840x2160
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 4096x2160
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//   [2]: 'NV12' (Y/CbCr 4:2:0)
//     Size: Discrete 640x480
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 640x360
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 1280x720
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)
//     Size: Discrete 1920x1080
//       Interval: Discrete 0.033s (30.000 fps)
//       Interval: Discrete 0.042s (24.000 fps)
//       Interval: Discrete 0.050s (20.000 fps)
//       Interval: Discrete 0.067s (15.000 fps)
//       Interval: Discrete 0.100s (10.000 fps)
//       Interval: Discrete 0.133s (7.500 fps)
//       Interval: Discrete 0.200s (5.000 fps)


V4L2_webcam_pipe::V4L2_webcam_pipe()
{
  
}

void V4L2_webcam_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);

  m_bus = bin->get_bus();
}

bool V4L2_webcam_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool V4L2_webcam_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool V4L2_webcam_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    //source
    m_src = Gst::ElementFactory::create_element("v4l2src", name);
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
    // m_src->add_probe(GST_PAD_PROBE_TYPE_IDLE | GST_PAD_PROBE_TYPE_EVENT_BOTH, sigc::mem_fun(&V4L2_webcam_pipe::on_pad_probe, this))

    //src caps
    m_src_caps = Gst::Caps::create_simple(
      "image/jpeg",
      "format","MJPG",
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "framerate",          Gst::Fraction(30, 1),
      "width",              3840,
      "height",             2160
      // "width",              1920,
      // "height",             1080

      );

    m_in_capsfilter = Gst::CapsFilter::create("incaps");
    m_in_capsfilter->property_caps() = m_src_caps;

    m_jpegparse    = Gst::ElementFactory::create_element("jpegparse");
    
    m_videorate    = Gst::ElementFactory::create_element("videorate");

    m_out_caps = Gst::Caps::create_simple(
      "image/jpeg",
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "framerate",         Gst::Fraction(30, 1)
      );

    m_out_capsfilter = Gst::CapsFilter::create("outcaps");
    m_out_capsfilter->property_caps() = m_out_caps;

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
    m_bin->add(m_in_capsfilter);
    // m_bin->add(m_jpegparse);
    m_bin->add(m_videorate);
    m_bin->add(m_out_capsfilter);
    m_bin->add(m_in_queue);
    m_bin->add(m_out_tee);
  }

  m_src->link(m_in_capsfilter);
  // m_in_capsfilter->link(m_jpegparse);
  // m_jpegparse->link(m_videorate);
  m_in_capsfilter->link(m_videorate);
  m_videorate->link(m_out_capsfilter);
  m_out_capsfilter->link(m_in_queue);
  m_in_queue->link(m_out_tee);
  
  // m_videoconvert->link(m_capsfilter);
  // m_capsfilter->link(m_out_queue);

  return true;
}

void V4L2_webcam_pipe::set_sensor_device(const char* dev_path)
{
    m_src->set_property("device", Glib::ustring(dev_path));
}

bool V4L2_webcam_pipe::set_exposure_mode(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_EXPOSURE_AUTO, val);
}
bool V4L2_webcam_pipe::get_exposure_mode(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_EXPOSURE_AUTO, val);
}
bool V4L2_webcam_pipe::set_exposure_value(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_EXPOSURE_ABSOLUTE, val);
}
bool V4L2_webcam_pipe::get_exposure_value(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_EXPOSURE_ABSOLUTE, val);
}

bool V4L2_webcam_pipe::set_focus_absolute(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_FOCUS_ABSOLUTE, val);
}
bool V4L2_webcam_pipe::get_focus_absolute(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_FOCUS_ABSOLUTE, val);
}

bool V4L2_webcam_pipe::set_focus_auto(bool val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_FOCUS_AUTO, val);
}
bool V4L2_webcam_pipe::get_focus_auto(bool* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_FOCUS_AUTO, val);
}

bool V4L2_webcam_pipe::set_brightness(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_BRIGHTNESS, val);
}
bool V4L2_webcam_pipe::get_brightness(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_BRIGHTNESS, val);
}

bool V4L2_webcam_pipe::set_gain(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_GAIN, val);
}
bool V4L2_webcam_pipe::get_gain(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_GAIN, val);
}