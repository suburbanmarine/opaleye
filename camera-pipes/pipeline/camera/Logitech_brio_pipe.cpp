#include "Logitech_brio_pipe.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

Logitech_brio_pipe::Logitech_brio_pipe() : m_gst_need_data(false)
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
      // "format", Gst::Fourcc(Glib::ustring("MJPG")),
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
    m_src->property_block() = false;
    m_src->property_min_latency()  = GST_SECOND / 30;

    m_src->property_emit_signals() = true;
    m_src->property_stream_type()  = Gst::APP_STREAM_TYPE_STREAM;
    // m_src->property_format()       = Gst::FORMAT_TIME;

    //get a cb per new frame
    m_camera.register_callback(std::bind(&Logitech_brio_pipe::new_frame_cb, this, std::placeholders::_1));

    // m_in_queue->property_max_size_buffers().set_value(2); // TODO buffer depth?

    m_src->signal_need_data().connect(
      [this](guint val){handle_need_data(val);}
      );
    m_src->signal_enough_data().connect(
      [this](){handle_enough_data();}
      );
    #if 0
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
  }

  //allocate a temp buffer
  //over allocate so we probably have enough for a 4k mjpeg
  m_frame_buffer = m_camera.allocate_frame(4096*4096*3);
  if( ! m_frame_buffer )
  {
    SPDLOG_ERROR("Could not alloc temp buffer");
    return false;
  }

  m_src->link(m_jpegparse);
  m_jpegparse->link(m_in_queue);
  m_in_queue->link(m_out_tee);
  
  // m_videoconvert->link(m_capsfilter);
  // m_capsfilter->link(m_out_queue);

  return true;
}

void Logitech_brio_pipe::new_frame_cb(uvc_frame_t* frame)
{
  // SPDLOG_ERROR("new frame {:d}: {:d}", frame->sequence, frame->data_bytes);

  {
    std::lock_guard<std::mutex> lock(m_frame_buffer_mutex);
    uvc_duplicate_frame(frame, m_frame_buffer.get());
  }

  if(m_gst_need_data)
  {
    Glib::RefPtr<Gst::Buffer> buf = Gst::Buffer::create(frame->data_bytes);
    gsize ins_len = buf->fill(0, frame->data, frame->data_bytes);
    
    if(ins_len != frame->data_bytes)
    {
      SPDLOG_ERROR("buffer did not accept all data");
    }

    // this used to be a problem - m_gst_need_data seems to help
    // do-timestamp=TRUE but buffers are provided before reaching the PLAYING state and having a clock. Timestamps will not be accurate!
    Gst::FlowReturn ret = m_src->push_buffer(buf);
    if(ret != Gst::FLOW_OK)
    {
      SPDLOG_ERROR("appsrc did not accept data"); 
    }
  }
}

void Logitech_brio_pipe::handle_need_data(guint val)
{
  SPDLOG_ERROR("handle_need_data");
  m_gst_need_data = true;
}
void Logitech_brio_pipe::handle_enough_data()
{
  SPDLOG_ERROR("handle_enough_data");
  m_gst_need_data = false;
}
#if 0
bool Logitech_brio_pipe::handle_seek_data(guint64 val)
{
  SPDLOG_ERROR("handle_seek_data {:d}", val);
  return false;
}
#endif
