/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "V4L2_alvium_pipe.hpp"
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
  reset();
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

bool V4L2_alvium_pipe::open(const char dev_path[])
{
  return m_cam->open("/dev/video0");
}
bool V4L2_alvium_pipe::close()
{
  m_frame_worker->interrupt();
  m_frame_worker->join();

  if( ! m_cam->close() )
  {
    SPDLOG_ERROR("close had error");
    return false;
  }

  return true;
}
bool V4L2_alvium_pipe::init(const char name[])
{

  m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

  // Bayer format (8/10/12 bit)
  // Pixel Format: 'RGGB' - V4L2_PIX_FMT_SRGGB8
  // Name        : 8-bit Bayer RGRG/GBGB
  // Pixel Format: 'JXR0' - v4l2_fourcc('J', 'X', 'R', '0')
  // Name        : 10-bit/16-bit Bayer RGRG/GBGB
  // Pixel Format: 'JXR2' - v4l2_fourcc('J', 'X', 'R', '2')
  // Name        : 12-bit/16-bit Bayer RGRG/GBGB
  
  // Luma Format (8bit)
  // Pixel Format: 'VYUY' - V4L2_PIX_FMT_VYUY
  // Name        : VYUY 4:2:2

  // Color Format (8bit)
  // Pixel Format: 'XR24' - V4L2_PIX_FMT_XBGR32
  // Name        : 32-bit BGRX 8-8-8-8

    m_cam = std::make_shared<Alvium_v4l2>();
    // m_cam->init("cam", v4l2_fourcc('J','X','Y','0')); // 12 bit gray
    m_cam->init("cam", v4l2_fourcc('X','R','2','4')); // 8 bit BGRX
    if( ! m_cam->set_free_trigger() ) 
    {
      SPDLOG_ERROR("Failed to set trigger mode");
    }
    // if( ! cam.set_hw_trigger(Alvium_CSI::v4l2_trigger_source::V4L2_TRIGGER_SOURCE_LINE0, Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_RISING_EDGE) ) // PDWN
    // {
    //   SPDLOG_ERROR("Failed to set trigger mode");
    // }

    //source caps
    //sometimes camera reports framerate as 16/1, sometimes 16593/1000
    //caps don't negotiate if framerate is wrong...
    // m_src_caps = Gst::Caps::create_from_string("video/x-raw, width=(int)2464, height=(int)2056, format=(string)BGRx, framerate=(fraction)16593/1000, pixel-aspect-ratio=(fraction)1/1");
    m_src_caps = Gst::Caps::create_from_string("video/x-raw, format=(string)BGRx, width=(int)2464, height=(int)2056, pixel-aspect-ratio=(fraction)1/1");
    if(! m_src_caps )
    {
      SPDLOG_ERROR("Failed to create m_src_caps");
    }

    //source
    m_src = Gst::AppSrc::create();
    m_src->property_caps().set_value(m_src_caps);

    m_src->property_is_live()      = true;
    m_src->property_do_timestamp() = true;
    // m_src->property_block()        = false;
    m_src->property_block()        = true; // TODO: this may need to be true to enable internal buffer
    m_src->property_min_latency()  = 0;
    m_src->property_max_latency()  = 1*GST_SECOND / 30;

    // m_src->property_num_buffers()  = 30;
    // m_src->property_max_bytes()    = 100*1024*1024;

    m_src->property_emit_signals() = false;
    m_src->property_stream_type()  = Gst::APP_STREAM_TYPE_STREAM;
    m_src->property_format()       = Gst::FORMAT_TIME;

    m_videorate    = Gst::ElementFactory::create_element("videorate");

    m_out_caps = Gst::Caps::create_from_string("video/x-raw, format=(string)BGRx, width=(int)2464, height=(int)2056, pixel-aspect-ratio=(fraction)1/1");

    m_out_capsfilter = Gst::CapsFilter::create("outcaps");
    if(! m_out_capsfilter )
    {
      SPDLOG_ERROR("Failed to create m_out_capsfilter");
      return false;
    }
    m_out_capsfilter->property_caps() = m_out_caps;

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
    m_bin->add(m_videorate);
    m_bin->add(m_out_capsfilter);
    m_bin->add(m_in_queue);
    m_bin->add(m_out_tee);

  m_src->link(m_videorate);
  m_videorate->link(m_out_capsfilter);
  m_out_capsfilter->link(m_in_queue);
  m_out_capsfilter->link(m_out_tee);

  m_frame_worker = std::make_shared<V4L2_alvium_frame_worker>(std::bind(&V4L2_alvium_pipe::new_frame_cb_XR24, this, std::placeholders::_1), m_cam);
  m_frame_worker->launch();

  return true;
}

void V4L2_alvium_pipe::handle_need_data(guint val)
{
  // SPDLOG_ERROR("handle_need_data");
  m_gst_need_data = true;
}
void V4L2_alvium_pipe::handle_enough_data()
{
  // SPDLOG_ERROR("handle_enough_data");
  m_gst_need_data = false;
}

void V4L2_alvium_pipe::new_frame_cb_XR24(const Alvium_v4l2::ConstMmapFramePtr& frame_buf)
{
  {
    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
    m_frame_buffer.assign((uint8_t const *)frame_buf->get_data(), (uint8_t const *)frame_buf->get_data() + frame_buf->get_bytes_used());
  }

  if(m_gst_need_data)
  {
    Glib::RefPtr<Gst::Buffer> buf = Gst::Buffer::create(frame_buf->get_bytes_used());
    gsize ins_len = buf->fill(0, frame_buf->get_data(), frame_buf->get_bytes_used());
    if(ins_len != frame_buf->get_bytes_used())
    {
      SPDLOG_ERROR("buffer did not accept all data");
    }

    // std::chrono::seconds      tv_sec(frame->capture_time.tv_sec);
    // std::chrono::microseconds tv_usec(frame->capture_time.tv_usec);
    // std::chrono::nanoseconds  pts_nsec = tv_sec + tv_usec;

    // buf->set_pts(m_curr_pts.count());
    // buf->set_duration(GST_SECOND / 30);

    // m_curr_pts += std::chrono::nanoseconds(GST_SECOND / 30);

    // this used to be a problem - m_gst_need_data seems to help
    // do-timestamp=TRUE but buffers are provided before reaching the PLAYING state and having a clock. Timestamps will not be accurate!
    Gst::FlowReturn ret = m_src->push_buffer(buf);
    if(ret != Gst::FLOW_OK)
    {
      SPDLOG_ERROR("appsrc did not accept data"); 
    }
  }
}
