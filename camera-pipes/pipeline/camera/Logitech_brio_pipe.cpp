#include "Logitech_brio_pipe.hpp"

#include "pipeline/gst_common.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

Logitech_brio_pipe::Logitech_brio_pipe() : m_gst_need_data(false)
{
  m_curr_pts = std::chrono::nanoseconds::zero();
}

// bool Logitech_brio_pipe::on_bus_message(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message)
void Logitech_brio_pipe::on_bus_message(const Glib::RefPtr<Gst::Message>& msg)
{
  using namespace Gst;
  if(msg)
  {
    SPDLOG_ERROR("Logitech_brio_pipe::on_bus_message {}", msg->get_message_type());
    switch(msg->get_message_type())
    {
      case GST_MESSAGE_UNKNOWN:
      {
        break;
      }
      case GST_MESSAGE_EOS:
      {
        break;
      }
      case GST_MESSAGE_ERROR:
      {
        break;
      }
      case GST_MESSAGE_WARNING:
      {
        break;
      }
      case GST_MESSAGE_INFO:
      {
        break;
      }
      case GST_MESSAGE_TAG:
      {
        break;
      }
      case GST_MESSAGE_BUFFERING:
      {
        break;
      }
      case GST_MESSAGE_STATE_CHANGED:
      {
        Gst::State state;
        Gst::State pending;
        Gst::ClockTime timeout = 0;
        Gst::StateChangeReturn ret = m_bin->get_state(state, pending, timeout);
        SPDLOG_ERROR("{} {}->{}", ret, state, pending);
        switch(state)
        {
          case GST_STATE_VOID_PENDING:
          {
            break;
          }
          case GST_STATE_NULL:
          {
            if( ! m_camera.stop() )
            {
              SPDLOG_ERROR("Could not stop camera");
            }

            m_gst_need_data = false;
            break;
          }
          case GST_STATE_READY:
          {
            break;
          }
          case GST_STATE_PAUSED:
          {
            if( ! m_camera.stop() )
            {
              SPDLOG_ERROR("Could not stop camera");
            }

            m_gst_need_data = false;
            break;
          }
          case GST_STATE_PLAYING:
          {
            if( ! m_camera.start() )
            {
             SPDLOG_ERROR("Could not start camera");
            }

            m_gst_need_data = true;

            break;
          }
        }
        break;
      }
      case GST_MESSAGE_STATE_DIRTY:
      {
        break;
      }
      case GST_MESSAGE_STEP_DONE:
      {
        break;
      }
      case GST_MESSAGE_CLOCK_PROVIDE:
      {
        break;
      }
      case GST_MESSAGE_CLOCK_LOST:
      {
        break;
      }
      case GST_MESSAGE_NEW_CLOCK:
      {
        break;
      }
      case GST_MESSAGE_STRUCTURE_CHANGE:
      {
        break;
      }
      case GST_MESSAGE_STREAM_STATUS:
      {
        break;
      }
      case GST_MESSAGE_APPLICATION:
      {
        break;
      }
      case GST_MESSAGE_ELEMENT:
      {
        break;
      }
      case GST_MESSAGE_SEGMENT_START:
      {
        break;
      }
      case GST_MESSAGE_SEGMENT_DONE:
      {
        break;
      }
      case GST_MESSAGE_DURATION_CHANGED:
      {
        break;
      }
      case GST_MESSAGE_LATENCY:
      {
        break;
      }
      case GST_MESSAGE_ASYNC_START:
      {
        break;
      }
      case GST_MESSAGE_ASYNC_DONE:
      {
        break;
      }
      case GST_MESSAGE_REQUEST_STATE:
      {
        break;
      }
      case GST_MESSAGE_STEP_START:
      {
        break;
      }
      case GST_MESSAGE_QOS:
      {
        break;
      }
      case GST_MESSAGE_PROGRESS:
      {
        break;
      }
      case GST_MESSAGE_TOC:
      {
        break;
      }
      case GST_MESSAGE_RESET_TIME:
      {
        break;
      }
      case GST_MESSAGE_STREAM_START:
      {
        break;
      }
      case GST_MESSAGE_NEED_CONTEXT:
      {
        break;
      }
      case GST_MESSAGE_HAVE_CONTEXT:
      {
        break;
      }
      case GST_MESSAGE_EXTENDED:
      {
        break;
      }
      // case GST_MESSAGE_DEVICE_ADDED:
      // case GST_MESSAGE_DEVICE_REMOVED:
      // case GST_MESSAGE_PROPERTY_NOTIFY:
      // case GST_MESSAGE_STREAM_COLLECTION:
      // case GST_MESSAGE_STREAMS_SELECTED:
      // case GST_MESSAGE_REDIRECT:
      // case GST_MESSAGE_DEVICE_CHANGED:
      // case GST_MESSAGE_INSTANT_RATE_REQUEST:
      // case GST_MESSAGE_ANY:
      default:
      {
        break;
      }
    }
  }
}

void Logitech_brio_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);

  m_bus = bin->get_bus();
  //connect a signal hanlder, this will be called in some gmaincontext
  m_bus->signal_message().connect(sigc::mem_fun(this, &Logitech_brio_pipe::on_bus_message));
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
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "format","MJPG",
      "framerate", Gst::Fraction(0, 1),
      "width",  640,
      "height", 480
      );

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

    //get a cb per new frame
    m_camera.register_callback(std::bind(&Logitech_brio_pipe::new_frame_cb, this, std::placeholders::_1));

    // m_in_queue->property_max_size_buffers().set_value(2); // TODO buffer depth?

    // m_src->signal_need_data().connect(
    //   [this](guint val){handle_need_data(val);}
    //   );
    // m_src->signal_enough_data().connect(
    //   [this](){handle_enough_data();}
    //   );
    // m_src->signal_seek_data().connect(
    //   [this](guint64 val){return handle_seek_data(val);}
    //   );

    m_jpegparse    = Gst::ElementFactory::create_element("jpegparse");
    
    m_videorate    = Gst::ElementFactory::create_element("videorate");

    m_out_caps = Gst::Caps::create_simple(
      "image/jpeg",
      "format","UYVY",
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      "framerate", Gst::Fraction(30, 1)
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
    m_bin->add(m_jpegparse);
    m_bin->add(m_videorate);
    m_bin->add(m_out_capsfilter);
    m_bin->add(m_in_queue);
    m_bin->add(m_out_tee);
  }

  if( ! m_camera.open() )
  {
   SPDLOG_ERROR("Could not open camera");
   return false;
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
  m_jpegparse->link(m_videorate);
  m_videorate->link(m_out_capsfilter);
  m_out_capsfilter->link(m_in_queue);
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

    // std::chrono::seconds      tv_sec(frame->capture_time.tv_sec);
    // std::chrono::microseconds tv_usec(frame->capture_time.tv_usec);
    // std::chrono::nanoseconds pts_nsec = tv_sec + tv_usec;

    // buf->set_pts(m_curr_pts.count());
    // buf->set_duration(GST_SECOND / 30);

    m_curr_pts += std::chrono::nanoseconds(GST_SECOND / 30);
    
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
  // SPDLOG_ERROR("handle_need_data");
  m_gst_need_data = true;
}
void Logitech_brio_pipe::handle_enough_data()
{
  // SPDLOG_ERROR("handle_enough_data");
  m_gst_need_data = false;
}
#if 0
bool Logitech_brio_pipe::handle_seek_data(guint64 val)
{
  SPDLOG_ERROR("handle_seek_data {:d}", val);
  return false;
}
#endif
