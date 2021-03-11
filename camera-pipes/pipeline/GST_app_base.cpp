#include "pipeline/GST_app_base.hpp"

GST_app_base::GST_app_base()
{
  m_pipeline     = Gst::Pipeline::create("pipeline");
  m_pipeline_bus = m_pipeline->get_bus();
  m_pipeline_bus->add_watch(sigc::ptr_fun(&on_bus_message));

  //glib event func
  m_mainloop_context = Glib::MainContext::create();
  m_mainloop         = Glib::MainLoop::create(m_mainloop_context, false);
}

GST_app_base::~GST_app_base()
{

}

bool GST_app_base::init()
{
  glib_main_thread_ = std::thread(&GST_app_base::run_glib_main, this);
  return true;
}

bool GST_app_base::start()
{
  m_pipeline->set_state(Gst::STATE_PLAYING);
  return true;
}
bool GST_app_base::stop()
{
  m_pipeline->set_state(Gst::STATE_NULL);
  m_mainloop->quit();
  glib_main_thread_.join();

  return true;
}

bool GST_app_base::on_bus_message(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message)
{
  switch(message->get_message_type())
  {
    case GST_MESSAGE_UNKNOWN:
    case GST_MESSAGE_EOS:
    case GST_MESSAGE_ERROR:
    case GST_MESSAGE_WARNING:
    case GST_MESSAGE_INFO:
    case GST_MESSAGE_TAG:
    case GST_MESSAGE_BUFFERING:
    case GST_MESSAGE_STATE_CHANGED:
    case GST_MESSAGE_STATE_DIRTY:
    case GST_MESSAGE_STEP_DONE:
    case GST_MESSAGE_CLOCK_PROVIDE:
    case GST_MESSAGE_CLOCK_LOST:
    case GST_MESSAGE_NEW_CLOCK:
    case GST_MESSAGE_STRUCTURE_CHANGE:
    case GST_MESSAGE_STREAM_STATUS:
    case GST_MESSAGE_APPLICATION:
    case GST_MESSAGE_ELEMENT:
    case GST_MESSAGE_SEGMENT_START:
    case GST_MESSAGE_SEGMENT_DONE:
    case GST_MESSAGE_DURATION_CHANGED:
    case GST_MESSAGE_LATENCY:
    case GST_MESSAGE_ASYNC_START:
    case GST_MESSAGE_ASYNC_DONE:
    case GST_MESSAGE_REQUEST_STATE:
    case GST_MESSAGE_STEP_START:
    case GST_MESSAGE_QOS:
    case GST_MESSAGE_PROGRESS:
    case GST_MESSAGE_TOC:
    case GST_MESSAGE_RESET_TIME:
    case GST_MESSAGE_STREAM_START:
    case GST_MESSAGE_NEED_CONTEXT:
    case GST_MESSAGE_HAVE_CONTEXT:
    case GST_MESSAGE_EXTENDED:
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
  return true;
}

void GST_app_base::make_debug_dot(const std::string& fname)
{
  Glib::RefPtr<Gst::Bin> bin = m_pipeline;
  gst_debug_bin_to_dot_file(bin->gobj(), GST_DEBUG_GRAPH_SHOW_ALL, fname.c_str());
}

void GST_app_base::make_debug_dot_ts(const std::string& fname)
{
  Glib::RefPtr<Gst::Bin> bin = m_pipeline;
  gst_debug_bin_to_dot_file_with_ts(bin->gobj(), GST_DEBUG_GRAPH_SHOW_ALL, fname.c_str());
}

void GST_app_base::run_glib_main()
{
  // for(;;)
  // {
    m_mainloop->run();
  // }
}