#pragma once

#include <glibmm/main.h>

#include <gstreamermm/element.h>
#include <gstreamermm/elementfactory.h>
#include <gstreamermm/bin.h>
#include <gstreamermm/pipeline.h>
#include <gstreamermm/bus.h>

#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

class GST_app_base
{
public:
  GST_app_base();

  virtual ~GST_app_base();

  virtual bool init();

  bool start();
  bool stop();

  // bool on_bus_message(const Glib::RefPtr<Gst::Bus>& bus, const Glib::RefPtr<Gst::Message>& message);
  void on_bus_message(const Glib::RefPtr<Gst::Message>& message);

  void make_debug_dot(const std::string& fname);

  void make_debug_dot_ts(const std::string& fname);

  void run_glib_main();

protected:
  Glib::RefPtr<Glib::MainContext>  m_mainloop_context;
  Glib::RefPtr<Glib::MainLoop>     m_mainloop;
  Glib::RefPtr<Gst::Pipeline>      m_pipeline;
  Glib::RefPtr<Gst::Bus>           m_pipeline_bus;

  std::thread glib_main_thread_;

  std::atomic<bool>       m_got_eos;
  std::mutex              m_mutex_got_eos;
  std::condition_variable m_cv_got_eos;

};