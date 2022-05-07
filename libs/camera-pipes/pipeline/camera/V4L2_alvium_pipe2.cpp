/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "V4L2_alvium_pipe2.hpp"

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

V4L2_alvium_pipe2::V4L2_alvium_pipe2() : m_gst_need_data(false)
{
  
}
V4L2_alvium_pipe2::~V4L2_alvium_pipe2()
{
  close();  
}

void V4L2_alvium_pipe2::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);

  m_bus = bin->get_bus();
}

bool V4L2_alvium_pipe2::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool V4L2_alvium_pipe2::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;

  // try
  // {
  //   m_out_tee->link(node);
  //   return true;
  // }
  // catch(const std::exception& e)
  // {
  //   SPDLOG_ERROR("Failed to link back: {:s}", e.what());
  // }
  // catch(...)
  // {
  //   SPDLOG_ERROR("Failed to link back, unknown exception"); 
  // }

  // return false;
}

bool V4L2_alvium_pipe2::close()
{

  return true;
}
bool V4L2_alvium_pipe2::init(const char name[])
{

  m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

  m_src_caps = Glib::wrap(gst_caps_new_simple ("video/x-raw",
       "format", G_TYPE_STRING, "RGB",
       "framerate", GST_TYPE_FRACTION, 20, 1,
       "max-framerate", GST_TYPE_FRACTION, 20, 1,
       "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
       "width", G_TYPE_INT, 1920,
       "height", G_TYPE_INT, 1080,
       "interlace-mode", G_TYPE_STRING, "progressive",
       // "colorimetry", G_TYPE_STRING, "sRGB",
       NULL));


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

    m_src->property_num_buffers()  = 300;
    m_src->property_max_bytes()    = 0;

    // m_src->property_emit_signals() = false;
    m_src->property_emit_signals() = true;
    m_src->property_stream_type()  = Gst::APP_STREAM_TYPE_STREAM;
    m_src->property_format()       = Gst::FORMAT_TIME;

    m_src->signal_need_data().connect(
      [this](guint val){handle_need_data(val);}
      );
    m_src->signal_enough_data().connect(
      [this](){handle_enough_data();}
      );
    // m_src->signal_seek_data().connect(
    //   [this](guint64 val){return handle_seek_data(val);}
    //   );

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
    m_in_queue->property_max_size_buffers()      = 300;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 0;

    
    m_vidconv = Gst::ElementFactory::create_element("videoconvert");

    //output tee
    m_out_tee = Gst::Tee::create();
    if(! m_out_tee )
    {
      SPDLOG_ERROR("Failed to create m_out_tee");
      return false;
    }

    m_bin->add(m_src);
    m_bin->add(m_in_queue);
    m_bin->add(m_vidconv);
    
    m_bin->add(m_out_tee);

  m_src->link(m_in_queue);
  m_in_queue->link(m_vidconv);
  m_vidconv->link(m_out_tee);


  return true;
}

void V4L2_alvium_pipe2::handle_need_data(guint val)
{
  // SPDLOG_ERROR("handle_need_data");
  m_gst_need_data = true;

  new_frame_cb_XR24();
}
void V4L2_alvium_pipe2::handle_enough_data()
{
  // SPDLOG_ERROR("handle_enough_data");
  m_gst_need_data = false;
}

void V4L2_alvium_pipe2::new_frame_cb_XR24()
{
  SPDLOG_DEBUG("V4L2_alvium_pipe2::new_frame_cb_XR24 - start");

  //todo put this in an object pool so we can share with zmq outgoing queue
  std::shared_ptr<std::vector<uint8_t>> new_frame = std::make_shared<std::vector<uint8_t>>(1920*1080*3);
  std::fill(new_frame->begin(), new_frame->end(), 0);

  //todo object pool for frame memory

  if(m_gst_need_data)
  {
    SPDLOG_DEBUG("feeding gst");
    Glib::RefPtr<Gst::Buffer> buf = Gst::Buffer::create(new_frame->size());
    Gst::MapInfo buf_map;
    if(!buf->map(buf_map, Gst::MAP_WRITE))
    {
      SPDLOG_ERROR("error mapping gstreamer buffer");
    }
    else
    {
      guint width  = 1920;
      guint height = 1080;
      gsize offset[1] = {0};
      gint xstride = 1920 * 3;
      gint stride[1] = {xstride};
      gst_buffer_add_video_meta_full(buf->gobj(), GST_VIDEO_FRAME_FLAG_NONE, GST_VIDEO_FORMAT_RGB, width, height, 1, offset, stride);

      // GST_BUFFER_FLAG_SET(buf->gobj(), GST_BUFFER_FLAG_LIVE);

      std::copy_n(new_frame->data(), new_frame->size(), buf_map.get_data());
      buf->unmap(buf_map);
     
      // gsize ins_len = buf->fill(0, frame_buf->get_data(), frame_buf->get_bytes_used());
      // if(ins_len != frame_buf->get_bytes_used())
      // {
      //   SPDLOG_ERROR("buffer did not accept all data");
      // }

      // buf->set_pts(timeval_to_chrono(frame_buf->get_buf()->timestamp).count());
      // buf->set_duration(GST_SECOND / 10);

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
  }

  //allocate new buffer and cache frame
  {
    
    {
      std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
      m_frame_buffer = new_frame;
    }
  }

  {
    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
    if(m_buffer_dispatch_cb)
    {
        m_buffer_dispatch_cb("", m_frame_buffer);
    }
  }

  SPDLOG_DEBUG("V4L2_alvium_pipe2::new_frame_cb_XR24 - end");
}
