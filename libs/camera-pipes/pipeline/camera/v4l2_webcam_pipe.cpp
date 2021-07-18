#include "v4l2_webcam_pipe.hpp"

#include "pipeline/gst_common.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

#include <sys/ioctl.h>

#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <set>
#include <map>

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
    // m_src->set_property("is-live", true);
    m_src->set_property("device", Glib::ustring("/dev/video0"));

	// (0): auto             - GST_V4L2_IO_AUTO
	// (1): rw               - GST_V4L2_IO_RW
	// (2): mmap             - GST_V4L2_IO_MMAP
	// (3): userptr          - GST_V4L2_IO_USERPTR
	// (4): dmabuf           - GST_V4L2_IO_DMABUF
	// (5): dmabuf-import    - GST_V4L2_IO_DMABUF_IMPORT
    m_src->set_property("io-mode", 2);
    // m_src->add_probe(GST_PAD_PROBE_TYPE_IDLE | GST_PAD_PROBE_TYPE_EVENT_BOTH, sigc::mem_fun(&V4L2_webcam_pipe::on_pad_probe, this))

    gint v4l2_fd;
	m_src->get_property("device-fd", v4l2_fd);

    //src caps
    m_src_caps = Gst::Caps::create_simple(
      "image/jpeg",
      "format","MJPG",
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "framerate",          Gst::Fraction(30, 1),
      // "width",              3840,
      // "height",             2160
      "width",              1920,
      "height",             1080

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

bool V4L2_webcam_pipe::v4l2_probe()
{
    gint v4l2_fd;
	m_src->get_property("device-fd", v4l2_fd);

	if(v4l2_fd < 0)
	{
		return false;
	}

	v4l2_capability cap;
	memset(&cap, 0, sizeof(cap));
	int ret = ioctl(v4l2_fd, VIDIOC_QUERYCAP, &cap);
	if(ret < 0)
	{
		SPDLOG_WARN("VIDIOC_QUERYCAP error: {:s}", m_errno.to_str());
		return false;
	}

	// The device supports the single-planar API through the Video Capture interface.
	//V4L2_CAP_VIDEO_CAPTURE
	// The device supports the multi-planar API through the Video Capture interface.
	//V4L2_CAP_VIDEO_CAPTURE_MPLANE
	// The device supports the single-planar API through the Video Output interface.
	//V4L2_CAP_VIDEO_OUTPUT
	// The device supports the multi-planar API through the Video Output interface.
	//V4L2_CAP_VIDEO_OUTPUT_MPLANE
	// The device supports the single-planar API through the Video Memory-To-Memory interface.
	//V4L2_CAP_VIDEO_M2M
	// The device supports the multi-planar API through the Video Memory-To-Memory interface.
	//V4L2_CAP_VIDEO_M2M_MPLANE
	// The device supports the Video Overlay interface. A video overlay device typically stores captured images directly in the video memory of a graphics card, with hardware clipping and scaling.
	//V4L2_CAP_VIDEO_OVERLAY
	// The device supports the Raw VBI Capture interface, providing Teletext and Closed Caption data.
	//V4L2_CAP_VBI_CAPTURE
	// The device supports the Raw VBI Output interface.
	//V4L2_CAP_VBI_OUTPUT
	// The device supports the Sliced VBI Capture interface.
	//V4L2_CAP_SLICED_VBI_CAPTURE
	// The device supports the Sliced VBI Output interface.
	//V4L2_CAP_SLICED_VBI_OUTPUT
	// The device supports the RDS capture interface.
	//V4L2_CAP_RDS_CAPTURE
	// The device supports the Video Output Overlay (OSD) interface. Unlike the Video Overlay interface, this is a secondary function of video output devices and overlays an image onto an outgoing video signal. When the driver sets this flag, it must clear the //V4L2_CAP_VIDEO_OVERLAY flag and vice versa. [1]
	//V4L2_CAP_VIDEO_OUTPUT_OVERLAY
	// The device supports the ioctl VIDIOC_S_HW_FREQ_SEEK ioctl for hardware frequency seeking.
	//V4L2_CAP_HW_FREQ_SEEK
	// The device supports the RDS output interface.
	//V4L2_CAP_RDS_OUTPUT
	// The device has some sort of tuner to receive RF-modulated video signals. For more information about tuner programming see Tuners and Modulators.
	//V4L2_CAP_TUNER
	// The device has audio inputs or outputs. It may or may not support audio recording or playback, in PCM or compressed formats. PCM audio support must be implemented as ALSA or OSS interface. For more information on audio inputs and outputs see Audio Inputs and Outputs.
	//V4L2_CAP_AUDIO
	// This is a radio receiver.
	//V4L2_CAP_RADIO
	// The device has some sort of modulator to emit RF-modulated video/audio signals. For more information about modulator programming see Tuners and Modulators.
	//V4L2_CAP_MODULATOR
	// The device supports the SDR Capture interface.
	//V4L2_CAP_SDR_CAPTURE
	// The device supports the struct v4l2_pix_format extended fields.
	//V4L2_CAP_EXT_PIX_FORMAT
	// The device supports the SDR Output interface.
	//V4L2_CAP_SDR_OUTPUT
	// The device supports the Metadata Interface capture interface.
	//V4L2_CAP_META_CAPTURE
	// The device supports the read() and/or write() I/O methods.
	//V4L2_CAP_READWRITE
	// The device supports the asynchronous I/O methods.
	//V4L2_CAP_ASYNCIO
	// The device supports the streaming I/O method.
	//V4L2_CAP_STREAMING
	// This is a touch device.
	//V4L2_CAP_TOUCH
	// The driver fills the device_caps field. This capability can only appear in the capabilities field and never in the device_caps field.
	//V4L2_CAP_DEVICE_CAPS
	if(cap.capabilities & V4L2_CAP_DEVICE_CAPS)
	{
		cap.device_caps;
	}

	std::map<uint32_t, v4l2_query_ext_ctrl> normal_ctrl;
	std::map<uint32_t, v4l2_query_ext_ctrl> menu_ctrl;

	v4l2_query_ext_ctrl ext_ctrl;
	uint32_t current_ctrl_id = V4L2_CID_BASE;
	do
	{
		memset(&ext_ctrl, 0, sizeof(ext_ctrl));
		ext_ctrl.id = current_ctrl_id; // query [V4L2_CID_BASE, V4L2_CID_LASTP1]
		ret = ioctl(v4l2_fd, VIDIOC_QUERY_EXT_CTRL, &ext_ctrl);
		if(ret < 0)
		{
			if(errno == EINVAL)
			{
				//were done
			}
			else
			{
				SPDLOG_WARN("VIDIOC_QUERY_EXT_CTRL error: {:s}", m_errno.to_str());
				return false;
			}
		}
		else
		{
			SPDLOG_DEBUG("VIDIOC_QUERY_EXT_CTRL {:d} {:s}, {:d}, [{:d}, {:d}]/{:d}",
				ext_ctrl.id,
				ext_ctrl.name,
				ext_ctrl.type,
				ext_ctrl.minimum,
				ext_ctrl.maximum,
				ext_ctrl.step);

			switch(ext_ctrl.type)
			{
				case V4L2_CTRL_TYPE_MENU:
				case V4L2_CTRL_TYPE_INTEGER_MENU:
				{
					menu_ctrl.insert(std::make_pair(ext_ctrl.id, ext_ctrl));
					break;
				}
				default:
				{
					normal_ctrl.insert(std::make_pair(ext_ctrl.id, ext_ctrl));	
					break;
				}
			}

			current_ctrl_id = ext_ctrl.id | V4L2_CTRL_FLAG_NEXT_CTRL;
		}
	} while(ret >= 0);

	std::map<uint32_t, std::set<int64_t>> menu_valid_entries;
	for(const auto& ctrl : menu_ctrl)
	{
		std::set<int64_t> menu_valid_entries_set_ref = menu_valid_entries[ctrl.second.id];

		v4l2_querymenu menu;
		for(int64_t i = ctrl.second.minimum; i <= ctrl.second.maximum; i += ctrl.second.step)
		{
			memset(&menu, 0, sizeof(menu));
			menu.id    = ctrl.second.id;
			menu.index = i;
	
			ret = ioctl(v4l2_fd, VIDIOC_QUERYMENU, &menu);
			if(ret < 0)
			{
				//menu entries may be sparse - so just note that this is invalid and keep scanning
				SPDLOG_WARN("VIDIOC_QUERYMENU {:d} {:s}[{:d}]: error: {:s}", 
					ctrl.second.id,
					ctrl.second.name,
					i,
					m_errno.to_str());
				continue;
			}
			else
			{
				menu_valid_entries_set_ref.insert(i);

				switch(ctrl.second.type)
				{
					case V4L2_CTRL_TYPE_MENU:
					{
						const uint8_t* name = menu.name;
						SPDLOG_DEBUG("VIDIOC_QUERYMENU {:d} {:s}[{:d}]: {:s}",
							ctrl.second.id,
							ctrl.second.name,
							i,
							name);
						break;
					}
					case V4L2_CTRL_TYPE_INTEGER_MENU:
					{
						int64_t value = menu.value;
						SPDLOG_DEBUG("VIDIOC_QUERYMENU {:d} {:s}[{:d}]: {:d}",
							ctrl.second.id,
							ctrl.second.name,
							i,
							value);
						break;
					}
					default:
					{
						SPDLOG_WARN("VIDIOC_QUERYMENU invalid type");
						return false;
					}
				}
			}
		}
	}


	// V4L2_CID_EXPOSURE_AUTO
	// V4L2_CID_EXPOSURE_AUTO_PRIORITY
	// V4L2_CID_EXPOSURE_ABSOLUTE

	return true;
}

bool V4L2_webcam_pipe::v4l2_ctrl_set(uint32_t id, const int32_t val)
{
    gint v4l2_fd;
	m_src->get_property("device-fd", v4l2_fd);

	v4l2_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id = id;
	ctrl.value = val;
	int ret = ioctl(v4l2_fd, VIDIOC_S_CTRL, &ctrl);	
	if(ret < 0)
	{
		SPDLOG_WARN("VIDIOC_S_CTRL error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}
bool V4L2_webcam_pipe::v4l2_ctrl_get(uint32_t id, int32_t* const out_val)
{
    gint v4l2_fd;
	m_src->get_property("device-fd", v4l2_fd);

	v4l2_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id = id;
	int ret = ioctl(v4l2_fd, VIDIOC_G_CTRL, &ctrl);	
	if(ret < 0)
	{
		SPDLOG_WARN("VIDIOC_S_CTRL error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = ctrl.value;
	return true;
}

bool V4L2_webcam_pipe::set_exposure_mode()
{
	return v4l2_ctrl_set(V4L2_CID_EXPOSURE_AUTO, 0);
}
bool V4L2_webcam_pipe::get_exposure_mode()
{
	return false;
}

bool V4L2_webcam_pipe::set_exposure_value()
{
	return false;
}
bool V4L2_webcam_pipe::get_exposure_value()
{
	return false;
}