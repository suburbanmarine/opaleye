/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "V4L2_alvium_pipe.hpp"

#include "pipeline/gst_common.hpp"

#include "opaleye-util/Ptree_util.hpp"

#include <boost/property_tree/ptree.hpp>

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>
#include <gst/video/video-format.h>
#include <gst/video/gstvideometa.h>

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

// User Controls

//                      brightness 0x00980900 (int)    : min=0 max=255 step=1 default=0 value=0 flags=slider
//                      saturation 0x00980902 (int)    : min=0 max=200 step=1 default=100 value=100 flags=slider
//                             hue 0x00980903 (int)    : min=-4000 max=4000 step=1 default=0 value=0 flags=slider
//              auto_white_balance 0x0098090c (bool)   : default=0 value=0
//                   white_balance 0x0098090d (button) : flags=write-only, execute-on-write
//                     red_balance 0x0098090e (int)    : min=0 max=8000 step=1 default=2354 value=2354 flags=slider
//                    blue_balance 0x0098090f (int)    : min=0 max=8000 step=1 default=2100 value=2100 flags=slider
//                           gamma 0x00980910 (int)    : min=40 max=240 step=5 default=100 value=100 flags=slider
//                        exposure 0x00980911 (int)    : min=126222 max=2147483647 step=56242 default=5019313 value=17617484 flags=slider
//                       gain_auto 0x00980912 (bool)   : default=0 value=1
//                            gain 0x00980913 (int)    : min=0 max=2400 step=10 default=0 value=330 flags=slider
//                       reverse_x 0x00980914 (bool)   : default=0 value=0
//                       reverse_y 0x00980915 (bool)   : default=0 value=0
//           frame_timeout_enabled 0x009809c8 (bool)   : default=1 value=1
//                   frame_timeout 0x009809c9 (int)    : min=100 max=12000 step=1 default=12000 value=12000
//        stride_alignment_enabled 0x009809ca (bool)   : default=1 value=1
//          crop_alignment_enabled 0x009809cb (bool)   : default=1 value=1

// Camera Controls

//                   exposure_auto 0x009a0901 (menu)   : min=0 max=1 default=1 value=0
//         0: Auto Mode
//         1: Manual Mode
//               exposure_absolute 0x009a0902 (int64)  : min=1 max=99999 step=1 default=1 value=176 flags=slider
//               exposure_auto_min 0x009a0928 (int)    : min=126222 max=2147483647 step=56242 default=175677 value=175677 flags=slider
//               exposure_auto_max 0x009a0929 (int)    : min=175677 max=2147483647 step=56242 default=1410066135 value=1410066135 flags=slider
//                   gain_auto_min 0x009a092a (int)    : min=0 max=2400 step=10 default=0 value=0 flags=slider
//                   gain_auto_max 0x009a092b (int)    : min=0 max=2400 step=10 default=2400 value=2400 flags=slider
//       exposure_active_line_mode 0x009a092c (bool)   : default=0 value=0
//   exposure_active_line_selector 0x009a092d (int)    : min=0 max=1 step=1 default=1 value=1 flags=slider
//          exposure_active_invert 0x009a092e (bool)   : default=0 value=0
//            sensor_configuration 0x009a2032 (u32)    : min=0 max=4294967295 step=1 default=0 [22] flags=read-only, volatile, has-payload
//          sensor_mode_i2c_packet 0x009a2033 (u32)    : min=0 max=4294967295 step=1 default=0 [1026] flags=read-only, volatile, has-payload
//       sensor_control_i2c_packet 0x009a2034 (u32)    : min=0 max=4294967295 step=1 default=0 [1026] flags=read-only, volatile, has-payload
//                     bypass_mode 0x009a2064 (intmenu): min=0 max=1 default=0 value=0
//         0: 0 (0x0)
//         1: 1 (0x1)
//                 override_enable 0x009a2065 (intmenu): min=0 max=1 default=0 value=0
//         0: 0 (0x0)
//         1: 1 (0x1)
//                    height_align 0x009a2066 (int)    : min=1 max=16 step=1 default=1 value=1
//                      size_align 0x009a2067 (intmenu): min=0 max=2 default=0 value=0
//         0: 1 (0x1)
//         1: 65536 (0x10000)
//         2: 131072 (0x20000)
//                write_isp_format 0x009a2068 (int)    : min=1 max=1 step=1 default=1 value=1
//        sensor_signal_properties 0x009a2069 (u32)    : min=0 max=4294967295 step=1 default=0 [30][18] flags=read-only, has-payload
//         sensor_image_properties 0x009a206a (u32)    : min=0 max=4294967295 step=1 default=0 [30][16] flags=read-only, has-payload
//       sensor_control_properties 0x009a206b (u32)    : min=0 max=4294967295 step=1 default=0 [30][36] flags=read-only, has-payload
//               sensor_dv_timings 0x009a206c (u32)    : min=0 max=4294967295 step=1 default=0 [30][16] flags=read-only, has-payload
//                low_latency_mode 0x009a206d (bool)   : default=0 value=0
//                preferred_stride 0x009a206e (int)    : min=0 max=65535 step=1 default=0 value=0
//                    sensor_modes 0x009a2082 (int)    : min=0 max=30 step=1 default=30 value=1 flags=read-only


void V4L2_alvium_frame_worker::work()
{
  while( ! is_interrupted() )
  {
    try
    {
      m_cam->wait_for_frame(std::chrono::milliseconds(250), m_cb);
    }
    catch(const std::exception& e)
    {
      SPDLOG_ERROR("V4L2_alvium_frame_worker::work: {:s}", e.what());
    }
    catch(...)
    {
      SPDLOG_ERROR("V4L2_alvium_frame_worker::work: unk exception");
    }
  }
}
/*
void V4L2_alvium_gst_worker::work()
{
  while( ! is_interrupted() )
  {
    if(m_cam_pipe->m_gst_need_data)
    {
      std::shared_ptr<std::vector<uint8_t>> frame_buf = m_cam_pipe->m_frame_buffer;
      
      Glib::RefPtr<Gst::Buffer> buf = Gst::Buffer::create(frame_buf->size());
      gsize ins_len = buf->fill(0, frame_buf->data(), frame_buf->size());
      if(ins_len != frame_buf->size())
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
      Gst::FlowReturn ret = m_cam_pipe->m_src->push_buffer(buf);
      if(ret != Gst::FLOW_OK)
      {
        SPDLOG_ERROR("appsrc did not accept data"); 
      }
    }
  }
}
*/
V4L2_alvium_pipe::V4L2_alvium_pipe() : m_gst_need_data(false)
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

void V4L2_alvium_pipe::set_params(const char dev_path[], const uint32_t fourcc, const std::string& trigger_mode)
{
  m_dev_path     = dev_path;
  m_fourcc       = fourcc;
  m_trigger_mode = trigger_mode;
}
bool V4L2_alvium_pipe::close()
{
  if(m_frame_worker)
  {
    m_frame_worker->interrupt();
    m_frame_worker->join();
  }

  if(m_cam)
  {
    m_cam->set_sw_trigger();
    m_cam->stop_streaming();
  
    if( ! m_cam->close() )
    {
      SPDLOG_ERROR("close had error");
      return false;
    }
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
    if( ! m_cam->open(m_dev_path.c_str()) )
    {
      SPDLOG_ERROR("Failed to open camera");
    }
    
    if( ! m_cam->init("cam", m_fourcc) )
    {
      SPDLOG_ERROR("Failed to init camera");
    }

    if(! set_trigger_mode(m_trigger_mode) )
    {
      SPDLOG_ERROR("Failed to set trigger mode");
      return false;
    }

    //source caps
    //sometimes camera reports framerate as 16/1, sometimes 16593/1000
    // m_src_caps = Gst::Caps::create_from_string("video/x-raw, format=BGRx, width=2464, height=2056");
    // m_src_caps = Gst::Caps::create_simple(
    //   "video/x-raw",
    //   "pixel-aspect-ratio", Gst::Fraction(1, 1),
    //   "format","BGRx",
    //   "framerate", Gst::Fraction(16, 1),
    //   "width",  2464,
    //   "height", 2056
    //   );

    switch(m_fourcc)
    {
      case v4l2_fourcc('J','X','R','0'):
      {
        m_src_caps = Glib::wrap(gst_caps_new_simple ("video/x-raw",
                   "format", G_TYPE_STRING, "GRAY16_LE",
                   "framerate", GST_TYPE_FRACTION, 0, 1,
                   "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                   "width", G_TYPE_INT, 2464,
                   "height", G_TYPE_INT, 2056,
                   NULL));
        break;
      }
      case v4l2_fourcc('J','X','R','2'):
      {
        m_src_caps = Glib::wrap(gst_caps_new_simple ("video/x-raw",
                   "format", G_TYPE_STRING, "GRAY16_LE",
                   "framerate", GST_TYPE_FRACTION, 0, 1,
                   "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                   "width", G_TYPE_INT, 2464,
                   "height", G_TYPE_INT, 2056,
                   NULL));
        break;
      }
      case v4l2_fourcc('J','X','Y','2'):
      {
        m_src_caps = Glib::wrap(gst_caps_new_simple ("video/x-raw",
                   "format", G_TYPE_STRING, "GRAY16_LE",
                   "framerate", GST_TYPE_FRACTION, 0, 1,
                   "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                   "width", G_TYPE_INT, 2464,
                   "height", G_TYPE_INT, 2056,
                   NULL));
        break;
      }
      case v4l2_fourcc('X','R','2','4'):
      {
        m_src_caps = Glib::wrap(gst_caps_new_simple ("video/x-raw",
             "format", G_TYPE_STRING, "BGRx",
             "framerate", GST_TYPE_FRACTION, 0, 1,
             "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
             "width", G_TYPE_INT, 2464,
             "height", G_TYPE_INT, 2056,
             NULL));
        break;
      }
      default:
      {
        SPDLOG_ERROR("unsupported fourcc");
        return false;
        break;
      }
    }

    if(! m_src_caps )
    {
      SPDLOG_ERROR("Failed to create m_src_caps");
      return false;
    }

    //source
#if 1
    m_src = Gst::AppSrc::create();
    m_src->property_caps().set_value(m_src_caps);

    m_src->property_is_live()      = true;
    m_src->property_do_timestamp() = true;
    m_src->property_block()        = false;
    // m_src->property_block()        = true; // TODO: this may need to be true to enable internal buffer
    m_src->property_min_latency()  = 0;
    m_src->property_max_latency()  = 1*GST_SECOND / 2;

    m_src->property_num_buffers()  = 30;
    m_src->property_max_bytes()    = 2464ULL*2056ULL*4ULL*10ULL;

    // m_src->property_emit_signals() = false;
    m_src->property_emit_signals() = true;
    m_src->property_stream_type()  = Gst::APP_STREAM_TYPE_STREAM;
    m_src->property_format()       = Gst::FORMAT_BYTES;

    m_src->signal_need_data().connect(
      [this](guint val){handle_need_data(val);}
      );
    m_src->signal_enough_data().connect(
      [this](){handle_enough_data();}
      );
    // m_src->signal_seek_data().connect(
    //   [this](guint64 val){return handle_seek_data(val);}
    //   );
#else
    m_src = Gst::AppSrc::create();
    m_src->property_caps().set_value(m_src_caps);
#endif
#if 1
    // m_videoconvert = Gst::ElementFactory::create_element("videoconvert");
    m_videoconvert = Gst::ElementFactory::create_element("nvvidconv");

    // m_videorate    = Gst::ElementFactory::create_element("videorate");

    // m_out_caps = Gst::Caps::create_from_string("video/x-raw, format=BGRx");
    // m_out_caps = Gst::Caps::create_simple(
    //   "video/x-raw",
    //   "format","BGRx"
    //   );

      // m_out_caps = Glib::wrap(gst_caps_new_simple ("video/x-raw(memory:NVMM)",
      //      "format", G_TYPE_STRING, "NV12",
      //      "framerate", GST_TYPE_FRACTION, 0, 1,
      //      "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
      //      "width", G_TYPE_INT, 2464,
      //      "height", G_TYPE_INT, 2056,
      //      NULL));

    m_out_caps = Glib::wrap(gst_caps_from_string("video/x-raw(memory:NVMM), width=(int)2464, height=(int)2056, format=(string)NV12"));

    if(! m_out_caps )
    {
      SPDLOG_ERROR("Failed to create m_out_caps");
      return false;
    }

    m_out_capsfilter = Gst::CapsFilter::create("outcaps");
    if(! m_out_capsfilter )
    {
      SPDLOG_ERROR("Failed to create m_out_capsfilter");
      return false;
    }
    m_out_capsfilter->property_caps().set_value(m_out_caps);

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
#endif
    // m_sink = Gst::FakeSink::create();

    m_bin->add(m_src);
    m_bin->add(m_videoconvert);
    m_bin->add(m_out_capsfilter);
    m_bin->add(m_in_queue);
    m_bin->add(m_out_tee);
    // m_bin->add(m_sink);

  m_src->link(m_videoconvert);
  m_videoconvert->link(m_out_capsfilter);
  m_out_capsfilter->link(m_in_queue);
  m_in_queue->link(m_out_tee);
  // m_out_tee->link(m_sink);

  switch(m_fourcc)
  {
    case v4l2_fourcc('J','X','R','0'):
    {
      m_frame_worker = std::make_shared<V4L2_alvium_frame_worker>(std::bind(&V4L2_alvium_pipe::new_frame_cb_JXR0, this, std::placeholders::_1), m_cam);
      break;
    }
    case v4l2_fourcc('J','X','R','2'):
    {
      m_frame_worker = std::make_shared<V4L2_alvium_frame_worker>(std::bind(&V4L2_alvium_pipe::new_frame_cb_JXR2, this, std::placeholders::_1), m_cam);
      break;
    }
    case v4l2_fourcc('J','X','Y','2'):
    {
      m_frame_worker = std::make_shared<V4L2_alvium_frame_worker>(std::bind(&V4L2_alvium_pipe::new_frame_cb_JXY2, this, std::placeholders::_1), m_cam);
      break;
    }
    case v4l2_fourcc('X','R','2','4'):
    {
      m_frame_worker = std::make_shared<V4L2_alvium_frame_worker>(std::bind(&V4L2_alvium_pipe::new_frame_cb_XR24, this, std::placeholders::_1), m_cam);
      break;
    }
    default:
    {
      SPDLOG_ERROR("unsupported fourcc");
      return false;
      break;
    }
  }
  
  m_frame_worker->launch();

  if( ! m_cam->start_streaming() )
  {
    SPDLOG_ERROR("m_cam.start_streaming() failed");
    return false;
  }

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

void V4L2_alvium_pipe::new_frame_cb_JXR0(const Alvium_v4l2::ConstMmapFramePtr& frame_buf)
{
  SPDLOG_TRACE("V4L2_alvium_pipe::new_frame_cb_JXR0 - start");

  boost::property_tree::ptree meta_tree;
  Alvium_v4l2::frame_meta_to_ptree(frame_buf, &meta_tree);
  std::string meta_str = Ptree_util::ptree_to_json_str(meta_tree);
  SPDLOG_TRACE("Metadata:\n{:s}", meta_str);

  //allocate new buffer and cache frame
  {
    //todo put this in an object pool so we can share with zmq outgoing queue
    std::shared_ptr<std::vector<uint8_t>> new_frame = std::make_shared<std::vector<uint8_t>>(frame_buf->get_bytes_used());
    new_frame->assign((uint8_t const *)frame_buf->get_data(), (uint8_t const *)frame_buf->get_data() + frame_buf->get_bytes_used());
    
    {
      std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
      m_frame_buffer = new_frame;

      if(m_buffer_dispatch_cb)
      {
          m_buffer_dispatch_cb(meta_str, m_frame_buffer);
      }
    }
  }

  SPDLOG_TRACE("V4L2_alvium_pipe::new_frame_cb_JXR0 - end");
}
void V4L2_alvium_pipe::new_frame_cb_JXR2(const Alvium_v4l2::ConstMmapFramePtr& frame_buf)
{
  SPDLOG_TRACE("V4L2_alvium_pipe::new_frame_cb_JXR2 - start");

  boost::property_tree::ptree meta_tree;
  Alvium_v4l2::frame_meta_to_ptree(frame_buf, &meta_tree);
  std::string meta_str = Ptree_util::ptree_to_json_str(meta_tree);
  SPDLOG_TRACE("Metadata:\n{:s}", meta_str);

  //allocate new buffer and cache frame
  {
    //todo put this in an object pool so we can share with zmq outgoing queue
    std::shared_ptr<std::vector<uint8_t>> new_frame = std::make_shared<std::vector<uint8_t>>(frame_buf->get_bytes_used());
    new_frame->assign((uint8_t const *)frame_buf->get_data(), (uint8_t const *)frame_buf->get_data() + frame_buf->get_bytes_used());
    
    {
      std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
      m_frame_buffer = new_frame;

      if(m_buffer_dispatch_cb)
      {
          m_buffer_dispatch_cb(meta_str, m_frame_buffer);
      }
    }
  }

  SPDLOG_TRACE("V4L2_alvium_pipe::new_frame_cb_JXR2 - end");
}
void V4L2_alvium_pipe::new_frame_cb_JXY2(const Alvium_v4l2::ConstMmapFramePtr& frame_buf)
{
  SPDLOG_TRACE("V4L2_alvium_pipe::new_frame_cb_JXY2 - start");

  boost::property_tree::ptree meta_tree;
  Alvium_v4l2::frame_meta_to_ptree(frame_buf, &meta_tree);
  std::string meta_str = Ptree_util::ptree_to_json_str(meta_tree);
  SPDLOG_TRACE("Metadata:\n{:s}", meta_str);

  //allocate new buffer and cache frame
  {
    //todo put this in an object pool so we can share with zmq outgoing queue
    std::shared_ptr<std::vector<uint8_t>> new_frame = std::make_shared<std::vector<uint8_t>>(frame_buf->get_bytes_used());
    new_frame->assign((uint8_t const *)frame_buf->get_data(), (uint8_t const *)frame_buf->get_data() + frame_buf->get_bytes_used());
    
    {
      std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
      m_frame_buffer = new_frame;

      if(m_buffer_dispatch_cb)
      {
          m_buffer_dispatch_cb(meta_str, m_frame_buffer);
      }
    }
  }

  SPDLOG_TRACE("V4L2_alvium_pipe::new_frame_cb_JXY2 - end");
}

void V4L2_alvium_pipe::new_frame_cb_XR24(const Alvium_v4l2::ConstMmapFramePtr& frame_buf)
{
  SPDLOG_TRACE("V4L2_alvium_pipe::new_frame_cb_XR24 - start");

  boost::property_tree::ptree meta_tree;
  Alvium_v4l2::frame_meta_to_ptree(frame_buf, &meta_tree);
  std::string meta_str = Ptree_util::ptree_to_json_str(meta_tree);
  SPDLOG_TRACE("Metadata:\n{:s}", meta_str);

  //allocate new buffer and cache frame
  {
    //todo put this in an object pool so we can share with zmq outgoing queue
    std::shared_ptr<std::vector<uint8_t>> new_frame = std::make_shared<std::vector<uint8_t>>(frame_buf->get_bytes_used());
    new_frame->assign((uint8_t const *)frame_buf->get_data(), (uint8_t const *)frame_buf->get_data() + frame_buf->get_bytes_used());
    
    {
      std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
      m_frame_buffer = new_frame;
    }
  }

  //todo send to zmq?
  //todo object pool for frame memory

  // if(false)
  if(m_gst_need_data)
  {
    SPDLOG_TRACE("feeding gst");
    Glib::RefPtr<Gst::Buffer> buf = Gst::Buffer::create(frame_buf->get_bytes_used());

    guint width  = 2464;
    guint height = 2056;
    gsize offset[1] = {0};
    gint xstride = 2464*4;
    gint stride[1] = {xstride};
    gst_buffer_add_video_meta_full(buf->gobj(), GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_FORMAT_BGRx, width, height, 1, offset, stride);
    GST_BUFFER_FLAG_SET(buf->gobj(), GST_BUFFER_FLAG_LIVE);

    Gst::MapInfo buf_map;
    buf->map(buf_map, Gst::MAP_WRITE);
    std::copy_n((uint8_t*)frame_buf->get_data(), frame_buf->get_bytes_used(), (uint8_t*)buf_map.get_data());
    buf->unmap(buf_map);

    // gsize ins_len = buf->fill(0, frame_buf->get_data(), frame_buf->get_bytes_used());
    // if(ins_len != frame_buf->get_bytes_used())
    // {
    //   SPDLOG_ERROR("buffer did not accept all data");
    // }

    // std::chrono::seconds      tv_sec(frame->capture_time.tv_sec);
    // std::chrono::microseconds tv_usec(frame->capture_time.tv_usec);
    // std::chrono::nanoseconds  pts_nsec = tv_sec + tv_usec;

    // buf->set_pts(m_curr_pts.count());
    // buf->set_duration(GST_SECOND / 30);

    // m_curr_pts += std::chrono::nanoseconds(GST_SECOND / 30);

    // this used to be a problem - m_gst_need_data seems to help
    // do-timestamp=TRUE but buffers are provided before reaching the PLAYING state and having a clock. Timestamps will not be accurate!
    Gst::FlowReturn ret = m_src->push_buffer(buf);

    // Glib::RefPtr<Gst::Caps>   m_buf_caps = Gst::Caps::create_from_string("video/x-raw, format=BGRx, width=2464, height=2056");
    // Glib::RefPtr<Gst::Sample> samp = Glib::wrap(gst_sample_new(buf->gobj(), m_buf_caps->gobj(), NULL, NULL));
    // Gst::FlowReturn ret = m_src->push_sample(samp);
    if(ret != Gst::FLOW_OK)
    {
      SPDLOG_ERROR("appsrc did not accept data"); 
    }
  }

  {
    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
    if(m_buffer_dispatch_cb)
    {
        m_buffer_dispatch_cb(meta_str, m_frame_buffer);
    }
  }

  SPDLOG_TRACE("V4L2_alvium_pipe::new_frame_cb_XR24 - end");
}

bool V4L2_alvium_pipe::set_camera_property(const std::string& property_id, const std::string& value)
{
  if(property_id == "streaming")
  {
    if(value == "on")
    {
      return start_streaming();
    }
    else if(value == "off")
    {
      return stop_streaming();
    }
  }
  else if(property_id == "trigger")
  {
    return set_trigger_mode(value);
  }

  return false;
}

bool V4L2_alvium_pipe::start_streaming()
{
  if( ! m_cam->start_streaming() )
  {
    SPDLOG_ERROR("m_cam.start_streaming() failed");
    return false;
  }

  return true;
}
bool V4L2_alvium_pipe::stop_streaming()
{
  if( ! m_cam->stop_streaming() )
  {
    SPDLOG_ERROR("m_cam.stop_streaming() failed");
    return false;
  }

  return true;
}
bool V4L2_alvium_pipe::set_trigger_mode(const std::string& mode)
{
  if(mode == "free")
  {
    if( ! m_cam->set_free_trigger() ) 
    {
      return false;
    }
    m_trigger_mode = mode;
  }
  else if(mode == "hw")
  {
    if( ! m_cam->set_hw_trigger(Alvium_CSI::v4l2_trigger_source::V4L2_TRIGGER_SOURCE_LINE0, Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_RISING_EDGE) ) 
    {
      return false;
    }
    m_trigger_mode = mode;
  }
  else if(mode == "sw")
  {
    if( ! m_cam->set_sw_trigger() ) 
    {
      return false;
    }
    m_trigger_mode = mode;
  }
  else
  {
    return false;
  }
  return true;
}
