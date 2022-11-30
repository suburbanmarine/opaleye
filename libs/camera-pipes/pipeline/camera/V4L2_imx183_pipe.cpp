/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "V4L2_imx183_pipe.hpp"

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
// vi-output, vc_mipi 2-001a (platform:15700000.vi:2):
//   /dev/video0
// 
// vi-output, vc_mipi 7-001a (platform:15700000.vi:4):
//   /dev/video1



// v4l2-ctl -d /dev/video0 --list-formats-ext
// ioctl: VIDIOC_ENUM_FMT
//   Index       : 0
//   Type        : Video Capture
//   Pixel Format: 'RG12'
//   Name        : 12-bit Bayer RGRG/GBGB
//     Size: Discrete 5440x3648
//       Interval: Discrete infs (0.000 fps)
// 

// Camera Controls
// 
//                      group_hold 0x009a2003 (bool)   : default=0 value=0 flags=execute-on-write
//                     sensor_mode 0x009a2008 (int64)  : min=0 max=1 step=1 default=0 value=0 flags=slider
//                            gain 0x009a2009 (int64)  : min=0 max=27000 step=26 default=0 value=0 flags=slider
//                        exposure 0x009a200a (int64)  : min=1 max=59652323 step=1 default=10000 value=1 flags=slider
//                      frame_rate 0x009a200b (int64)  : min=0 max=11202 step=1 default=0 value=0 flags=slider
//                    trigger_mode 0x009a200e (int)    : min=0 max=7 step=1 default=0 value=1
//                         io_mode 0x009a200f (int)    : min=0 max=5 step=1 default=0 value=3
//                     black_level 0x009a2010 (int)    : min=0 max=2147483647 step=1 default=0 value=0 flags=slider
//                  single_trigger 0x009a2011 (button) : flags=write-only, execute-on-write
//                     bypass_mode 0x009a2064 (intmenu): min=0 max=1 default=0 value=0
//                 override_enable 0x009a2065 (intmenu): min=0 max=1 default=0 value=0
//                    height_align 0x009a2066 (int)    : min=1 max=16 step=1 default=1 value=1
//                      size_align 0x009a2067 (intmenu): min=0 max=2 default=0 value=0
//                write_isp_format 0x009a2068 (int)    : min=1 max=1 step=1 default=1 value=1
//        sensor_signal_properties 0x009a2069 (u32)    : min=0 max=4294967295 step=1 default=0 [30][18] flags=read-only, has-payload
//         sensor_image_properties 0x009a206a (u32)    : min=0 max=4294967295 step=1 default=0 [30][18] flags=read-only, has-payload
//       sensor_control_properties 0x009a206b (u32)    : min=0 max=4294967295 step=1 default=0 [30][36] flags=read-only, has-payload
//               sensor_dv_timings 0x009a206c (u32)    : min=0 max=4294967295 step=1 default=0 [30][16] flags=read-only, has-payload
//                low_latency_mode 0x009a206d (bool)   : default=0 value=0
//                preferred_stride 0x009a206e (int)    : min=0 max=65535 step=1 default=0 value=0
//                    sensor_modes 0x009a2082 (int)    : min=0 max=30 step=1 default=30 value=1 flags=read-only



void V4L2_imx183_frame_worker::work()
{
  while( ! is_interrupted() )
  {
    try
    {
      m_cam->wait_for_frame(std::chrono::milliseconds(250), m_cb);
    }
    catch(const std::exception& e)
    {
      SPDLOG_ERROR("V4L2_imx183_frame_worker::work: {:s}", e.what());
    }
    catch(...)
    {
      SPDLOG_ERROR("V4L2_imx183_frame_worker::work: unk exception");
    }
  }
}

V4L2_imx183_pipe::V4L2_imx183_pipe() : m_gst_need_data(false)
{
  m_curr_pts = std::chrono::nanoseconds::zero();
}
V4L2_imx183_pipe::~V4L2_imx183_pipe()
{
  close();  
}

void V4L2_imx183_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);

  m_bus = bin->get_bus();
}

bool V4L2_imx183_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool V4L2_imx183_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  try
  {
    m_out_tee->link(node);
  }
  catch(const std::exception& e)
  {
    SPDLOG_ERROR("Failed to link back: {:s}", e.what());
    return false;
  }
  catch(...)
  {
    SPDLOG_ERROR("Failed to link back, unknown exception"); 
    return false;
  }

  return true;
}

void V4L2_imx183_pipe::set_params(const char dev_path[], const uint32_t fourcc, const std::string& trigger_mode)
{
  m_dev_path     = dev_path;
  m_fourcc       = fourcc;
  m_trigger_mode = trigger_mode;
}
bool V4L2_imx183_pipe::close()
{
  if(m_cam)
  {
    m_cam->stop_streaming();
    set_trigger_mode("free");
  }

  if(m_frame_worker)
  {
    m_frame_worker->interrupt();
    m_frame_worker->join();

    m_frame_worker.reset();
  }

  if(m_cam)
  {
    if( ! m_cam->close() )
    {
      SPDLOG_ERROR("close had error");
      return false;
    }

    m_cam.reset();
  }


  return true;
}
bool V4L2_imx183_pipe::init(const char name[])
{

  m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    m_cam = std::make_shared<imx183_v4l2>();
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

    m_v4l2_util.set_fd(m_cam->get_fd());

    set_gain_auto(true);
    set_exposure_auto(0);

#if 0
    switch(m_fourcc)
    {
      case v4l2_fourcc('J','X','R','0'):
      {
        m_src_caps = Glib::wrap(gst_caps_new_simple ("video/x-raw",
                   "format", G_TYPE_STRING, "GRAY16_LE",
                   "framerate", GST_TYPE_FRACTION, 0, 1,
                   "max-framerate", GST_TYPE_FRACTION, 20, 1,
                   "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                   "width", G_TYPE_INT, 2464,
                   "height", G_TYPE_INT, 2056,
                   "interlace-mode", G_TYPE_STRING, "progressive",
                   "colorimetry", G_TYPE_STRING, "sRGB",
                   NULL));
        break;
      }
      case v4l2_fourcc('J','X','R','2'):
      {
        m_src_caps = Glib::wrap(gst_caps_new_simple ("video/x-raw",
                   "format", G_TYPE_STRING, "GRAY16_LE",
                   "framerate", GST_TYPE_FRACTION, 0, 1,
                   "max-framerate", GST_TYPE_FRACTION, 20, 1,
                   "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                   "width", G_TYPE_INT, 2464,
                   "height", G_TYPE_INT, 2056,
                   "interlace-mode", G_TYPE_STRING, "progressive",
                   "colorimetry", G_TYPE_STRING, "sRGB",
                   NULL));
        break;
      }
      case v4l2_fourcc('J','X','Y','2'):
      {
        m_src_caps = Glib::wrap(gst_caps_new_simple ("video/x-raw",
                   "format", G_TYPE_STRING, "GRAY16_LE",
                   "framerate", GST_TYPE_FRACTION, 0, 1,
                   "max-framerate", GST_TYPE_FRACTION, 20, 1,
                   "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                   "width", G_TYPE_INT, 2464,
                   "height", G_TYPE_INT, 2056,
                   "interlace-mode", G_TYPE_STRING, "progressive",
                   "colorimetry", G_TYPE_STRING, "sRGB",
                   NULL));
        break;
      }
      case v4l2_fourcc('X','R','2','4'):
      {
        m_src_caps = Glib::wrap(gst_caps_new_simple ("video/x-raw",
             "format", G_TYPE_STRING, "BGRx",
             "framerate", GST_TYPE_FRACTION, 0, 1,
             "max-framerate", GST_TYPE_FRACTION, 20, 1,
             "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
             "width", G_TYPE_INT, 2464,
             "height", G_TYPE_INT, 2056,
             "interlace-mode", G_TYPE_STRING, "progressive",
             "colorimetry", G_TYPE_STRING, "sRGB",
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
    m_src->property_format()       = Gst::FORMAT_DEFAULT;

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
    //https://lists.freedesktop.org/archives/gstreamer-devel/2016-February/056621.html
  m_appsrc = gst_element_factory_make("appsrc", NULL);
  g_object_set(m_appsrc, "is-live",      TRUE, NULL);
  g_object_set(m_appsrc, "do-timestamp", TRUE, NULL);
  g_object_set(m_appsrc, "block",        FALSE, NULL);
  g_object_set(m_appsrc, "min-latency",  GST_SECOND / 20LL, NULL);
  // g_object_set(m_appsrc, "max-latency",  GST_SECOND / 2LL, NULL);
  // g_object_set(m_appsrc, "num-buffers",  10, NULL);
  // g_object_set(m_appsrc, "max-bytes",    2464ULL*2056ULL*4ULL*10ULL, NULL);
  g_object_set(m_appsrc, "emit-signals", FALSE, NULL);
  g_object_set(m_appsrc, "stream-type",  GST_APP_STREAM_TYPE_STREAM, NULL);
  g_object_set(m_appsrc, "format",       GST_FORMAT_TIME, NULL);
  // g_object_set(m_appsrc, "leaky-type",   GST_APP_LEAKY_TYPE_DOWNSTREAM, NULL); // DNE on xnx

  switch(m_fourcc)
  {
    case PIX_FMT_RG12:
    {
      m_src_caps= gst_caps_from_string("video/x-raw, format=(string)GRAY16_LE, framerate=0/1, max-framerate=20/1, pixel-aspect-ratio=1/1, width=5440, height=3648, interlace-mode=progressive, colorimetry=sRGB");
      break;
    }
    default:
    {
      SPDLOG_ERROR("unsupported fourcc");
      return false;
      break;
    }
  }
  assert(m_src_caps);
  gst_app_src_set_caps(GST_APP_SRC(m_appsrc), m_src_caps);

#endif

    m_in_queue     = Gst::Queue::create();
    if(! m_in_queue )
    {
      SPDLOG_ERROR("Failed to create m_in_queue");
      return false;
    }

    m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    m_in_queue->property_max_size_buffers()      = 15;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 0;

    m_grayconvert            = Gst::ElementFactory::create_element("videoconvert");
    m_grayconvert_caps       = Glib::wrap(gst_caps_from_string("video/x-raw, format=(string)GRAY8"));
    m_grayconvert_capsfilter = Gst::CapsFilter::create("graycaps");
    m_grayconvert_capsfilter->property_caps().set_value(m_grayconvert_caps);

    m_videoconvert = Gst::ElementFactory::create_element("nvvidconv");

    m_out_caps = Glib::wrap(gst_caps_from_string("video/x-raw(memory:NVMM), format=(string)NV12"));
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

    m_out_queue     = Gst::Queue::create();
    if(! m_out_queue )
    {
      SPDLOG_ERROR("Failed to create m_out_queue");
      return false;
    }

    m_out_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    m_out_queue->property_max_size_buffers()      = 15;
    m_out_queue->property_max_size_bytes()        = 0;
    m_out_queue->property_max_size_time()         = 0;

    //output tee
    m_out_tee = Gst::Tee::create();
    if(! m_out_tee )
    {
      SPDLOG_ERROR("Failed to create m_out_tee");
      return false;
    }

    gst_bin_add(GST_BIN(m_bin->gobj()), m_appsrc);
    m_bin->add(m_in_queue);
    m_bin->add(m_grayconvert);
    m_bin->add(m_grayconvert_capsfilter);

    m_bin->add(m_videoconvert);
    m_bin->add(m_out_capsfilter);
    m_bin->add(m_out_queue);
    m_bin->add(m_out_tee);

  Glib::RefPtr<Gst::Element> m_in_queue_element = m_in_queue;
  gst_element_link(m_appsrc, m_in_queue_element->gobj());
  m_in_queue->link(m_grayconvert);
  m_grayconvert->link(m_grayconvert_capsfilter);
  m_grayconvert_capsfilter->link(m_videoconvert);
  m_videoconvert->link(m_out_capsfilter);
  m_out_capsfilter->link(m_out_queue);
  m_out_queue->link(m_out_tee);

  switch(m_fourcc)
  {
    case PIX_FMT_RG12:
    {
      m_frame_worker = std::make_shared<V4L2_imx183_frame_worker>(std::bind(&V4L2_imx183_pipe::new_frame_cb_RG12, this, std::placeholders::_1), m_cam);
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

void V4L2_imx183_pipe::handle_need_data(guint val)
{
  // SPDLOG_ERROR("handle_need_data");
  m_gst_need_data = true;
}
void V4L2_imx183_pipe::handle_enough_data()
{
  // SPDLOG_ERROR("handle_enough_data");
  m_gst_need_data = false;
}

void V4L2_imx183_pipe::new_frame_cb_RG12(const v4l2_base::ConstMmapFramePtr& frame_buf)
{
  SPDLOG_TRACE("V4L2_imx183_pipe::new_frame_cb_RG12 - start");

  boost::property_tree::ptree meta_tree;
  v4l2_base::frame_meta_to_ptree(frame_buf, &meta_tree);
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

  #if 0
  {
    // cv::COLOR_BayerRGGB2RGB
    // cv::COLOR_BayerBG2RGB
    cv::Mat in(frame_buf->get_fmt().fmt.pix.height, frame_buf->get_fmt().fmt.pix.width, CV_16UC1, frame_buf->get_data());
    cv::Mat out;
    cv::demosaicing(in, out, cv::COLOR_BayerBG2BGR, 3);
  }
  #endif

  #if 1
  {
    SPDLOG_DEBUG("feeding gst");

    GstBuffer* buf = gst_buffer_new_and_alloc(frame_buf->get_bytes_used());
    GstMapInfo buf_map;
    gst_buffer_map(buf, &buf_map, GST_MAP_WRITE);

    std::copy_n((uint8_t*)frame_buf->get_data(), frame_buf->get_bytes_used(), (uint8_t*)buf_map.data);

    guint width  = frame_buf->get_fmt().fmt.pix.width;
    guint height = frame_buf->get_fmt().fmt.pix.height;
    gsize offset[1] = {0};
    gint xstride = frame_buf->get_fmt().fmt.pix.width * 4;
    gint stride[1] = {xstride};
    
    gst_buffer_add_video_meta_full(buf, GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_FORMAT_GRAY16_LE, width, height, 1, offset, stride);
    // gst_buffer_add_video_meta_full(buf, GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_FORMAT_GRAY8, width, height, 1, offset, stride);
    
    // GST_BUFFER_FLAG_SET(buf, GST_BUFFER_FLAG_LIVE);
    // GST_BUFFER_PTS(buf)      = m_curr_pts.count();
    // GST_BUFFER_DURATION(buf) = GST_SECOND / 10L;
    // m_curr_pts += std::chrono::nanoseconds(GST_SECOND / 10L);
    
    gst_buffer_unmap (buf, &buf_map);

    gst_app_src_push_buffer(GST_APP_SRC(m_appsrc), buf);

    // Glib::RefPtr<Gst::Caps>   m_buf_caps = Gst::Caps::create_from_string("video/x-raw, format=BGRx, width=2464, height=2056");
    // Glib::RefPtr<Gst::Sample> samp = Glib::wrap(gst_sample_new(buf->gobj(), m_buf_caps->gobj(), NULL, NULL));
    // Gst::FlowReturn ret = m_src->push_sample(samp);
    // if(ret != Gst::FLOW_OK)
    // {
    //   SPDLOG_ERROR("appsrc did not accept data"); 
    // }
  }
  #endif

  {
    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
    if(m_buffer_dispatch_cb)
    {
        m_buffer_dispatch_cb(meta_str, m_frame_buffer);
    }
  }

  SPDLOG_TRACE("V4L2_imx183_pipe::new_frame_cb_RG12 - end");
}

bool V4L2_imx183_pipe::set_camera_property(const std::string& property_id, const std::string& value)
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

bool V4L2_imx183_pipe::start_streaming()
{
  if( ! m_cam->start_streaming() )
  {
    SPDLOG_ERROR("m_cam.start_streaming() failed");
    return false;
  }

  return true;
}
bool V4L2_imx183_pipe::stop_streaming()
{
  if( ! m_cam->stop_streaming() )
  {
    SPDLOG_ERROR("m_cam.stop_streaming() failed");
    return false;
  }

  return true;
}
bool V4L2_imx183_pipe::set_trigger_mode(const std::string& mode)
{
  const std::optional<uint32_t> io_mode      = m_cam->get_v4l2_util().get_ext_ctrl_id_by_name("IO Mode");
  const std::optional<uint32_t> trigger_mode = m_cam->get_v4l2_util().get_ext_ctrl_id_by_name("Trigger Mode");
  if(! (io_mode.has_value() && trigger_mode.has_value()))
  {
    return false;
  }

  if(mode == "free")
  {
    m_cam->get_v4l2_util().v4l2_ctrl_set(trigger_mode.value(), 0);
    m_cam->get_v4l2_util().v4l2_ctrl_set(io_mode.value(), 0);
    m_trigger_mode = mode;
  }
  else if(mode == "hw")
  {
    m_cam->get_v4l2_util().v4l2_ctrl_set(io_mode.value(), 3);
    m_cam->get_v4l2_util().v4l2_ctrl_set(trigger_mode.value(), 1);
    m_trigger_mode = mode;
  }
  else
  {
    return false;
  }
  return true;
}
