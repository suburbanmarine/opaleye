/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "gst_app_mjpeg.hpp"

#include "pipeline/encode/h264_swenc_bin.hpp"
#include "pipeline/decode/jpeg_swdec_bin.hpp"

#include "pipeline/encode/h264_nvenc_bin.hpp"
#include "pipeline/decode/jpeg_nvv4l2decoder_bin.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

test_app_mjpeg::test_app_mjpeg()
{

}

test_app_mjpeg::~test_app_mjpeg()
{

}

bool test_app_mjpeg::init()
{
  if ( ! GST_app_base::init() )
  {
    SPDLOG_ERROR("Could not init GST_app_base");
    return false;
  }

  if(!m_config)
  {
    m_config = std::make_shared<app_config>();
    m_config->make_default();
  }

  if(m_config->h264_mode == "nv")
  {
    SPDLOG_INFO("NV mode");
    m_thumb  = std::make_shared<Thumbnail_nv_pipe>();
  }
  else
  {
    SPDLOG_INFO("CPU mode");

    m_thumb  = std::make_shared<Thumbnail_sw_pipe>();
    
  }

  if( ! m_camera.init("cam_0") )
  {
   SPDLOG_ERROR("Could not init camera");
   return false;
  }

  if( ! m_thumb->init("thumb_0") )
  {
   SPDLOG_ERROR("Could not init thumb");
   return false;
  }

  if( ! m_stream_interpipesink.init("stream_ipsink_0") )
  {
   SPDLOG_ERROR("Could not init stream interpipe");
   return false;
  }

  if( ! m_rtppay.init("rtp_0") )
  {
   SPDLOG_ERROR("Could not init m_rtp");
   return false;
  }

  if( ! m_rtpsink.init("udp_0") )
  {
   SPDLOG_ERROR("Could not init m_udp");
   return false;
  }

  //add elements to top level bin
  m_camera.add_to_bin(m_pipeline);
  m_thumb->add_to_bin(m_pipeline);
  m_stream_interpipesink.add_to_bin(m_pipeline);
  m_rtppay.add_to_bin(m_pipeline);
  m_rtpsink.add_to_bin(m_pipeline);

  //link pipeline
  m_camera.link_back(m_thumb->front());
  m_camera.link_back(m_stream_interpipesink.front());
  m_camera.link_back(m_rtppay.front());

  m_rtppay.link_back(m_rtpsink.front());

  return true;
}
bool test_app_mjpeg::start_camera(const std::string& camera)
{
  SPDLOG_INFO("test_app_mjpeg::start_camera({:s})", camera);
  return true;
}
bool test_app_mjpeg::stop_camera(const std::string& camera)
{
  SPDLOG_INFO("test_app_mjpeg::stop_camera({:s})", camera);
  return true;
}

bool test_app_mjpeg::start_video_capture(const std::string& camera)
{
  SPDLOG_INFO("test_app_mjpeg::start_video_capture({:s})", camera);

  if(m_mkv_pipe)
  {
    return false;
  }

  m_mkv_pipe = std::make_shared<gst_filesink_pipeline>();
  m_mkv_pipe->set_top_storage_dir(m_config->video_path.string());
  if(m_mkv_pipe->init())
  {
    m_mkv_pipe->set_listen_to("stream_ipsink_0");
    if(m_mkv_pipe->start())
    {
      return true;
    }
    else
    {
      SPDLOG_INFO("m_mkv_pipe start failed");

      m_mkv_pipe.reset();
      return false;   
    }
  }
  else
  {
    SPDLOG_INFO("m_mkv_pipe init failed");
    m_mkv_pipe.reset();
    return false;
  }

  return true;
}
bool test_app_mjpeg::stop_video_capture(const std::string& camera)
{
  SPDLOG_INFO("test_app_mjpeg::stop_video_capture({:s})", camera);

  if(!m_mkv_pipe)
  {
    return false;
  }

  m_mkv_pipe->set_listen_to(NULL);
  m_mkv_pipe->send_pipeline_eos();
  m_mkv_pipe->wait_pipeline_eos();
  m_mkv_pipe->stop();
  m_mkv_pipe.reset();

  return true;
}

bool test_app_mjpeg::start_still_capture(const std::string& camera)
{
  SPDLOG_INFO("test_app_mjpeg::start_still_capture({:s})", camera);
  return true;
}
bool test_app_mjpeg::stop_still_capture(const std::string& camera)
{
  SPDLOG_INFO("test_app_mjpeg::stop_still_capture({:s})", camera);
  return true;
}

bool test_app_mjpeg::start_rtp_stream(const std::string& ip_addr, int port)
{
  SPDLOG_INFO("test_app_mjpeg::start_rtp_stream {:s}:{:d}", ip_addr, port);

  if( (port < 0) || (port > 65535))
  {
    throw std::domain_error("port must be in [0, 65535]");
  }

  return m_rtpsink.add_udp_client(ip_addr, port);
}
bool test_app_mjpeg::stop_rtp_stream(const std::string& ip_addr, int port)
{
  SPDLOG_INFO("test_app_mjpeg::stop_rtp_stream {:s}:{:d}", ip_addr, port);

  if( (port < 0) || (port > 65535))
  {
    throw std::domain_error("port must be in [0, 65535]");
  }

  return m_rtpsink.remove_udp_client(ip_addr, port);
}
bool test_app_mjpeg::stop_rtp_all_stream()
{
  SPDLOG_INFO("test_app_mjpeg::stop_rtp_all_stream");
  return false;
}
std::string test_app_mjpeg::get_pipeline_status()
{
  SPDLOG_INFO("test_app_mjpeg::get_pipeline_status");
  Glib::RefPtr<Gst::Bin> bin = m_pipeline;
  Gst::State state;
  Gst::State pending_state;
  Gst::StateChangeReturn ret = bin->get_state(state, pending_state, 0);

  char const * ret_str = "CHANGE_UNKNOWN";
  switch(ret)
  {
    case GST_STATE_CHANGE_FAILURE:
    {
      ret_str = "CHANGE_FAILURE";
      break;
    }
    case GST_STATE_CHANGE_SUCCESS:
    {
      ret_str = "CHANGE_SUCCESS";
      break;
    }
    case GST_STATE_CHANGE_ASYNC:
    {
      ret_str = "CHANGE_ASYNC";
      break;
    }
    case GST_STATE_CHANGE_NO_PREROLL:
    {
      ret_str = "CHANGE_NO_PREROLL";
      break;
    }
    default:
    {
     ret_str =  "CHANGE_UNKNOWN";
     break;
    }
  }

  char const * state_str = "UNKNOWN";
  switch(state)
  {
    case GST_STATE_VOID_PENDING:
    {
      state_str = "VOID_PENDING";
      break;
    }
    case GST_STATE_NULL:
    {
      state_str = "NULL";
      break;
    }
    case GST_STATE_READY:
    {
      state_str = "READY";
      break;
    }
    case GST_STATE_PAUSED:
    {
      state_str = "PAUSED";
      break;
    }
    case GST_STATE_PLAYING:
    {
      state_str = "PLAYING";
      break;
    }
    default:
    {
     state_str =  "UNKNOWN";
     break;
    }
  }

  return fmt::format("{:s}-{:s}", state_str, ret_str);
}
std::string test_app_mjpeg::get_pipeline_graph()
{
  SPDLOG_INFO("test_app_mjpeg::get_pipeline_graph");
  
  make_debug_dot("pipeline");
  int ret = system("dot -Tpdf -o /tmp/pipeline.dot.pdf /tmp/pipeline.dot");
  if(ret == -1)
  {
    SPDLOG_ERROR("Could not create pdf");
  }

  if(m_mkv_pipe)
  {
    m_mkv_pipe->make_debug_dot("pipeline_mkv");
    int ret = system("dot -Tpdf -o /tmp/pipeline_mkv.dot.pdf /tmp/pipeline_mkv.dot");
    if(ret == -1)
    {
      SPDLOG_ERROR("Could not create pdf");
    }
  }

  return std::string();
}
void test_app_mjpeg::set_config(const std::string& config)
{
  SPDLOG_INFO("test_app_mjpeg::set_config()");
}
std::string test_app_mjpeg::get_config() const
{
  SPDLOG_INFO("test_app_mjpeg::get_config");
  return std::string();
}

void test_app_mjpeg::set_default_config()
{
  SPDLOG_INFO("test_app_mjpeg::set_default_config");
}

void test_app_mjpeg::restart_software()
{
  SPDLOG_INFO("test_app_mjpeg::restart_software");
}
void test_app_mjpeg::reboot()
{
  SPDLOG_INFO("test_app_mjpeg::reboot");
}
void test_app_mjpeg::shutdown()
{
  SPDLOG_INFO("test_app_mjpeg::shutdown");
}

std::vector<std::string> test_app_mjpeg::get_camera_list() const
{
  SPDLOG_INFO("test_app_mjpeg::get_camera_list");
  return std::vector<std::string>();
}

bool test_app_mjpeg::set_camera_property(const std::string& camera_id, const std::string& property_id, const std::string& value)
{
  bool ret = false;
  if(camera_id == "cam0")
  {
    if(property_id == "exposure_mode")
    {
      // ret = m_camera.set_exposure_mode();
    }
    else if(property_id == "exposure_absolute")
    {
     // ret = m_camera.set_exposure_value(); 
    }
    else
    {
      ret = false; 
    }
  }
  else
  {
    ret = false; 
  }

  return ret;
}