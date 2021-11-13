/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "Opaleye_app.hpp"

#include "pipeline/encode/h264_swenc_bin.hpp"
#include "pipeline/decode/jpeg_swdec_bin.hpp"

#include "pipeline/encode/h264_nvenc_bin.hpp"
#include "pipeline/decode/jpeg_nvdec_bin.hpp"
#include "pipeline/decode/jpeg_nvv4l2decoder_bin.hpp"

#include "pipeline/Thumbnail_sw2_pipe.hpp"
#include "pipeline/Thumbnail_nv_pipe.hpp"

#include <boost/lexical_cast.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

Gstreamer_pipeline::Gstreamer_pipeline()
{

}

Gstreamer_pipeline::~Gstreamer_pipeline()
{

}

bool Gstreamer_pipeline::init()
{
  if ( ! GST_app_base::init() )
  {
    SPDLOG_ERROR("Could not init GST_app_base");
    return false;
  }

  return true;
}

bool Gstreamer_pipeline::make_pipeline(const std::shared_ptr<const app_config>& app_config, const pipeline_config& pipe_config)
{
  m_app_config = app_config;
  m_pipeline_config = pipe_config;

  bool ret = false;

  if(m_pipeline_config.type == "brio")
  {
    ret = make_brio_pipeline();
  }
  else if(m_pipeline_config.type == "imx219")
  {
    make_imx219_pipeline();
  }
  else if(m_pipeline_config.type == "virtual")
  {
    make_virtual_pipeline();
  }
  else
  {
    SPDLOG_ERROR("Unknown Pipeline Type");
  }

  return ret;
}

bool Gstreamer_pipeline::make_brio_pipeline()
{
  #if 1
  if(m_app_config->h264_mode == "nv")
  {
    SPDLOG_INFO("NV mode");
    // https://forums.developer.nvidia.com/t/bus-error-with-gstreamer-and-opencv/110657/5
    // libjpeg and nvjpegdec may not be used in the same program...
    // m_jpgdec = std::make_shared<jpeg_nvdec_pipe>();
    // m_jpgdec = std::make_shared<jpeg_swdec_bin>();
    m_jpgdec = std::make_shared<jpeg_nvv4l2decoder_bin>();
    m_h264   = std::make_shared<h264_nvenc_bin>();
    m_thumb  = std::make_shared<Thumbnail_nv_pipe>();
  }
  else
  {
    SPDLOG_INFO("CPU mode");

    m_jpgdec = std::make_shared<jpeg_swdec_bin>();
    m_h264   = std::make_shared<h264_swenc_bin>();
    m_thumb  = std::make_shared<Thumbnail_sw_pipe>();
    
  }

  // if( ! m_test_src.init("cam_1") )
  // {
  //  SPDLOG_ERROR("Could not init camera");
  //  return false;
  // }

  if( ! m_camera.init("cam_0") )
  {
   SPDLOG_ERROR("Could not init camera");
   return false;
  }

  if( ! m_jpgdec->init("jpgdec_0") )
  {
   SPDLOG_ERROR("Could not init jpgdec");
   return false;
  }

  if( ! m_thumb->init("thumb_0") )
  {
   SPDLOG_ERROR("Could not init thumb");
   return false;
  }

  if( ! m_h264->init("h264_0") )
  {
   SPDLOG_ERROR("Could not init h264");
   return false;
  }

  if( ! m_h264_interpipesink.init("h264_ipsink_0") )
  {
   SPDLOG_ERROR("Could not init h264 interpipe");
   return false;
  }
  
  // if( ! m_mkv.init("mkv_0") )
  // {
  //  SPDLOG_ERROR("Could not init mkv");
  //  return false;
  // }

  // if( ! m_display.init("display_0") )
  // {
  //  SPDLOG_ERROR("Could not init m_display");
  //  return false;
  // }

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
  m_jpgdec->add_to_bin(m_pipeline);
  // m_test_src.add_to_bin(m_pipeline);
  m_thumb->add_to_bin(m_pipeline);
  m_h264->add_to_bin(m_pipeline);
  m_h264_interpipesink.add_to_bin(m_pipeline);
  // m_mkv.add_to_bin(m_pipeline);
  // m_display.add_to_bin(m_pipeline);
  m_rtppay.add_to_bin(m_pipeline);
  m_rtpsink.add_to_bin(m_pipeline);

  //link pipeline
  m_camera.link_back(m_jpgdec->front());
  m_camera.link_back(m_thumb->front());

  // m_jpgdec->link_back(m_display.front());
  m_jpgdec->link_back(m_h264->front());

  // m_test_src.link_back(m_display.front());
  // m_test_src.link_back(m_h264->front());

  m_h264->link_back(m_rtppay.front());
  m_h264->link_back(m_h264_interpipesink.front());

  m_rtppay.link_back(m_rtpsink.front());
  #endif

  return true;
}

bool Gstreamer_pipeline::make_imx219_pipeline()
{
  return true;
}

bool Gstreamer_pipeline::make_virtual_pipeline()
{
  #if 1
  if(m_app_config->h264_mode == "nv")
  {
    SPDLOG_INFO("NV mode");
    // https://forums.developer.nvidia.com/t/bus-error-with-gstreamer-and-opencv/110657/5
    // libjpeg and nvjpegdec may not be used in the same program...
    // m_jpgdec = std::make_shared<jpeg_nvdec_pipe>();
    // m_jpgdec = std::make_shared<jpeg_swdec_bin>();
    m_jpgdec = std::make_shared<jpeg_nvv4l2decoder_bin>();
    m_h264   = std::make_shared<h264_nvenc_bin>();
    m_thumb  = std::make_shared<Thumbnail_sw_pipe>();
  }
  else
  {
    SPDLOG_INFO("CPU mode");

    m_jpgdec = std::make_shared<jpeg_swdec_bin>();
    m_h264   = std::make_shared<h264_swenc_bin>();
    m_thumb  = std::make_shared<Thumbnail_sw_pipe>();
  }

  if( ! m_test_src.init("cam_0") )
  {
   SPDLOG_ERROR("Could not init camera");
   return false;
  }

  if( ! m_thumb->init("thumb_0") )
  {
   SPDLOG_ERROR("Could not init thumb");
   return false;
  }

  if( ! m_h264->init("h264_0") )
  {
   SPDLOG_ERROR("Could not init h264");
   return false;
  }

  if( ! m_h264_interpipesink.init("h264_ipsink_0") )
  {
   SPDLOG_ERROR("Could not init h264 interpipe");
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
  m_test_src.add_to_bin(m_pipeline);

  m_thumb->add_to_bin(m_pipeline);
  m_h264->add_to_bin(m_pipeline);
  m_h264_interpipesink.add_to_bin(m_pipeline);

  m_rtppay.add_to_bin(m_pipeline);
  m_rtpsink.add_to_bin(m_pipeline);

  //link pipeline
  m_test_src.link_back(m_thumb->front());

  m_test_src.link_back(m_h264->front());

  m_h264->link_back(m_rtppay.front());
  m_h264->link_back(m_h264_interpipesink.front());

  m_rtppay.link_back(m_rtpsink.front());
  #endif

  return true;
}

Opaleye_app::Opaleye_app()
{

}

Opaleye_app::~Opaleye_app()
{

}

bool Opaleye_app::init()
{
  if(!m_config)
  {
    m_config = std::make_shared<app_config>();
    m_config->make_default();
  }

  {
    std::shared_ptr<Gstreamer_pipeline> pipeline = std::make_shared<Gstreamer_pipeline>();
    if( ! pipeline->init() )
    {
      return false;
    }

    if( ! pipeline->make_pipeline(m_config, m_config->camera_configs["cam0"].pipeline) )
    {
      return false;
    }
    
    m_pipelines["pipe0"] = pipeline;
  }

  {
    std::shared_ptr<Gstreamer_pipeline> pipeline = std::make_shared<Gstreamer_pipeline>();
    if( ! pipeline->init() )
    {
      return false;
    }

    if( ! pipeline->make_pipeline(m_config, m_config->camera_configs["cam1"].pipeline) )
    {
      return false;
    }
    
    m_pipelines["pipe1"] = pipeline;
  }

  return true;
}
bool Opaleye_app::start_camera(const std::string& camera)
{
  SPDLOG_INFO("Opaleye_app::start_camera({:s})", camera);
  return true;
}
bool Opaleye_app::stop_camera(const std::string& camera)
{
  SPDLOG_INFO("Opaleye_app::stop_camera({:s})", camera);
  return true;
}

bool Opaleye_app::start_video_capture(const std::string& camera)
{
  SPDLOG_INFO("Opaleye_app::start_video_capture({:s})", camera);

  std::shared_ptr<gst_filesink_pipeline> m_mkv_pipe = m_pipelines["pipe0"]->m_mkv_pipe;

  if(m_mkv_pipe)
  {
    return false;
  }

  m_mkv_pipe = std::make_shared<gst_filesink_pipeline>();
  m_mkv_pipe->set_top_storage_dir(m_config->video_path.string());
  if(m_mkv_pipe->init())
  {
    m_mkv_pipe->set_listen_to("h264_ipsink_0");
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
bool Opaleye_app::stop_video_capture(const std::string& camera)
{
  SPDLOG_INFO("Opaleye_app::stop_video_capture({:s})", camera);

  std::shared_ptr<gst_filesink_pipeline> m_mkv_pipe = m_pipelines["pipe0"]->m_mkv_pipe;

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

bool Opaleye_app::start_still_capture(const std::string& camera)
{
  SPDLOG_INFO("Opaleye_app::start_still_capture({:s})", camera);
  return true;
}
bool Opaleye_app::stop_still_capture(const std::string& camera)
{
  SPDLOG_INFO("Opaleye_app::stop_still_capture({:s})", camera);
  return true;
}

bool Opaleye_app::start_rtp_stream(const std::string& ip_addr, int port)
{
  SPDLOG_INFO("Opaleye_app::start_rtp_stream {:s}:{:d}", ip_addr, port);

  if( (port < 0) || (port > 65535))
  {
    throw std::domain_error("port must be in [0, 65535]");
  }

  rtpsink_pipe& m_rtpsink = m_pipelines["pipe0"]->m_rtpsink;

  return m_rtpsink.add_udp_client(ip_addr, port);
}
bool Opaleye_app::stop_rtp_stream(const std::string& ip_addr, int port)
{
  SPDLOG_INFO("Opaleye_app::stop_rtp_stream {:s}:{:d}", ip_addr, port);

  if( (port < 0) || (port > 65535))
  {
    throw std::domain_error("port must be in [0, 65535]");
  }

  rtpsink_pipe& m_rtpsink = m_pipelines["pipe0"]->m_rtpsink;

  return m_rtpsink.remove_udp_client(ip_addr, port);
}
bool Opaleye_app::stop_rtp_all_stream()
{
  SPDLOG_INFO("Opaleye_app::stop_rtp_all_stream");
  return false;
}

bool Opaleye_app::start()
{
  SPDLOG_INFO("Opaleye_app::start");
  
  bool ret = true;

  for(auto& v : m_pipelines)
  {
    ret = ret && v.second->start();
  }

  return ret;
}
bool Opaleye_app::stop()
{
  SPDLOG_INFO("Opaleye_app::stop");

  bool ret = true;

  for(auto& v : m_pipelines)
  {
    ret = ret && v.second->stop();
  }

  return ret;
}

std::string Opaleye_app::get_pipeline_status()
{
  SPDLOG_INFO("Opaleye_app::get_pipeline_status");
  Glib::RefPtr<Gst::Bin> bin = m_pipelines["pipe0"]->get_pipeline();
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
std::string Opaleye_app::get_pipeline_graph()
{
  SPDLOG_INFO("Opaleye_app::get_pipeline_graph");
  
  m_pipelines["pipe0"]->make_debug_dot("pipeline");
  int ret = system("dot -Tpdf -o /tmp/pipeline.dot.pdf /tmp/pipeline.dot");
  if(ret == -1)
  {
    SPDLOG_ERROR("Could not create pdf");
  }

  std::shared_ptr<gst_filesink_pipeline> m_mkv_pipe = m_pipelines["pipe0"]->m_mkv_pipe;
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
void Opaleye_app::set_config(const std::string& config)
{
  SPDLOG_INFO("Opaleye_app::set_config()");
}
std::string Opaleye_app::get_config() const
{
  SPDLOG_INFO("Opaleye_app::get_config");
  return std::string();
}

void Opaleye_app::set_default_config()
{
  SPDLOG_INFO("Opaleye_app::set_default_config");
}

void Opaleye_app::restart_software()
{
  SPDLOG_INFO("Opaleye_app::restart_software");
}
void Opaleye_app::reboot()
{
  SPDLOG_INFO("Opaleye_app::reboot");
}
void Opaleye_app::shutdown()
{
  SPDLOG_INFO("Opaleye_app::shutdown");
}

std::vector<std::string> Opaleye_app::get_camera_list() const
{
  SPDLOG_INFO("Opaleye_app::get_camera_list");
  return std::vector<std::string>();
}

bool Opaleye_app::set_camera_property(const std::string& camera_id, const std::string& property_id, int value)
{
  V4L2_webcam_pipe& m_camera = m_pipelines["pipe0"]->m_camera;

  bool ret = false;
  if(camera_id == "cam0")
  {
    if(property_id == "exposure_mode")
    {
      ret = m_camera.set_exposure_mode(value); 
    }
    else if(property_id == "exposure_absolute")
    {
     ret = m_camera.set_exposure_value(value);
    }
    else if(property_id == "focus_absolute")
    {
      ret = m_camera.set_focus_absolute(value);
    }
    else if(property_id == "focus_auto")
    {
      ret = m_camera.set_focus_auto(value);
    }
    else if(property_id == "brightness")
    {
      ret = m_camera.set_brightness(value);
    }
    else if(property_id == "gain")
    {
      ret = m_camera.set_gain(value);
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