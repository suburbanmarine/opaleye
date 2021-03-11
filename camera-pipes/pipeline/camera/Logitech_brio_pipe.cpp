#include "Logitech_brio_pipe.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

Logitech_brio_pipe::Logitech_brio_pipe()
{
  
}

void Logitech_brio_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}
bool Logitech_brio_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool Logitech_brio_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool Logitech_brio_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    //src caps
    m_src_caps = Gst::Caps::create_simple(
      "video/x-jpeg",
      // "format", Gst::Fourcc('M', 'J', 'P', 'G'),
      // "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "framerate", Gst::Fraction(30, 1),
      "width",  640,
      "height", 480
      );

    //source
    m_src = Gst::AppSrc::create();
    m_src->property_caps().set_value(m_src_caps);

    m_src->property_is_live()      = true;
    m_src->property_do_timestamp() = true;
    // m_src->property_num_buffers()  = 2; // TODO buffer depth?
    m_src->property_min_latency()  = GST_SECOND / 30;

    m_src->property_emit_signals() = false;
    m_src->property_stream_type()  = Gst::APP_STREAM_TYPE_STREAM;
    // m_src->property_format()       = Gst::FORMAT_TIME;

    //get a cb per new frame
    m_camera.register_callback(std::bind(&Logitech_brio_pipe::new_frame_cb, this, std::placeholders::_1));

    // m_in_queue->property_max_size_buffers().set_value(2); // TODO buffer depth?

    //TODO - it looks like we don't get this event for live streams
    //just register a callback and push data in when it gets here
    #if 0
    m_src->signal_need_data().connect(
      [this](guint val){handle_need_data(val);}
      );
    m_src->signal_enough_data().connect(
      [this](){handle_enough_data();}
      );
    m_src->signal_seek_data().connect(
      [this](guint64 val){return handle_seek_data(val);}
      );
    #endif

    m_in_queue     = Gst::Queue::create();
    m_jpegparse    = Gst::ElementFactory::create_element("jpegparse");

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_src);
    m_bin->add(m_jpegparse);
    m_bin->add(m_in_queue);
    m_bin->add(m_out_tee);
  }

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

  //allocate a temp buffer
  // m_frame_buffer = m_camera.copy_front_buffer();
  // if( ! m_frame_buffer )
  // {
  //   SPDLOG_ERROR("Could not alloc temp buffer");
  //   return false;
  // }

  m_src->link(m_jpegparse);
  m_jpegparse->link(m_in_queue);
  m_in_queue->link(m_out_tee);
  
  // m_videoconvert->link(m_capsfilter);
  // m_capsfilter->link(m_out_queue);

  return true;
}

void Logitech_brio_pipe::new_frame_cb(const std::shared_ptr<uvc_frame_t>& frame)
{
  #if 1
  // SPDLOG_ERROR("new frame {:d}: {:d}", frame->sequence, frame->data_bytes);

  Glib::RefPtr<Gst::Buffer> buf = Gst::Buffer::create(frame->data_bytes);
  gsize ins_len = buf->fill(0, frame->data, frame->data_bytes);
  
  if(ins_len != frame->data_bytes)
  {
    SPDLOG_ERROR("buffer did not accept all data");
  }

  Gst::FlowReturn ret = m_src->push_buffer(buf);
  if(ret != Gst::FLOW_OK)
  {
    SPDLOG_ERROR("appsrc did not accept data"); 
  }
  #else
  uvc_duplicate_frame(frame.get(), m_frame_buffer.get());
  #endif
}
#if 0
void Logitech_brio_pipe::handle_need_data(guint val)
{
  SPDLOG_ERROR("handle_need_data");

  if(m_camera.copy_front_buffer(m_frame_buffer))
  {
    Glib::RefPtr<Gst::Buffer> buf = Gst::Buffer::create(m_frame_buffer->data_bytes);
    gsize ins_len = buf->fill(0, m_frame_buffer->data, m_frame_buffer->data_bytes);
    GST_BUFFER_FLAG_SET(buf->gobj(), GST_BUFFER_FLAG_LIVE);
    
    if(ins_len != m_frame_buffer->data_bytes)
    {
      SPDLOG_ERROR("buffer did not accept all data");
    }

    Gst::FlowReturn ret = m_src->push_buffer(buf);
    if(ret != Gst::FLOW_OK)
    {
      SPDLOG_ERROR("appsrc did not accept data"); 
    }
    // Glib::RefPtr<Gst::Caps> frame_caps = Gst::Caps::create_simple(
    //   "video/x-jpeg",
    //   // "format", Gst::Fourcc('M', 'J', 'P', 'G'),
    //   "pixel-aspect-ratio", Gst::Fraction(1, 1),
    //   "framerate", Gst::Fraction(30, 1),
    //   "width",  m_frame_buffer->width,
    //   "height", m_frame_buffer->height
    //   );
    // Glib::RefPtr<Gst::Sample> sample = Gst::Sample::create(buf, frame_caps, const Gst::Segment& segment, Gst::Structure&& info);

    // Gst::FlowReturn ret = m_src->push_sample(sample);
  }
  else
  {
    //no new buffer!
    SPDLOG_ERROR("camera did not have buffer for appsrc"); 
  }


  // Gst::FlowReturn push_buffer(const Glib::RefPtr<Gst::Buffer>& arg0);
  // Gst::FlowReturn push_sample(const Glib::RefPtr<Gst::Sample>& arg0);
}
void Logitech_brio_pipe::handle_enough_data()
{
  SPDLOG_ERROR("handhandle_enough_datale_need_data");
}
bool Logitech_brio_pipe::handle_seek_data(guint64 val)
{
  SPDLOG_ERROR("handle_seek_data {:d}", val);
  return false;
}
#endif
