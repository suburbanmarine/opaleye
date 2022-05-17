/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "Opaleye_app.hpp"
#include "gst_framesink_pipeline.hpp"

#include "pipeline/encode/h264_swenc_bin.hpp"
#include "pipeline/decode/jpeg_swdec_bin.hpp"

#include "pipeline/encode/h264_nvenc_bin.hpp"
#include "pipeline/decode/jpeg_nvdec_bin.hpp"
#include "pipeline/decode/jpeg_nvv4l2decoder_bin.hpp"

#include "pipeline/Thumbnail_sw2_pipe.hpp"
#include "pipeline/Thumbnail_nv_pipe.hpp"
#include "pipeline/Thumbnail_nv3_pipe.hpp"

#include "pipeline/GST_fakesink.hpp"
#include "pipeline/nvvideoconvert_pipe.hpp"

#include "pipeline/timecodestamper.hpp"
#include "pipeline/timeoverlay.hpp"

#include <boost/lexical_cast.hpp>

#include <jsonrpc-lean/fault.h>

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

bool Gstreamer_pipeline::make_pipeline(const std::shared_ptr<const app_config>& app_config, const camera_config& camera_config, const pipeline_config& pipe_config)
{
  m_app_config      = app_config;
  m_camera_config   = camera_config;
  m_pipeline_config = pipe_config;

  bool ret = false;

  if(m_pipeline_config.type == "brio")
  {
    ret = make_brio_pipeline();
  }
  else if(m_pipeline_config.type == "imx219")
  {
    ret = make_imx219_pipeline();
  }
  else if(m_pipeline_config.type == "alvium")
  {
    ret = make_alvium_pipeline();
  }
  else if(m_pipeline_config.type == "virtual")
  {
    ret = make_virtual_pipeline();
  }
  else
  {
    SPDLOG_ERROR("Unknown Pipeline Type");
  }

  return ret;
}

std::shared_ptr<GST_element_base> Gstreamer_pipeline::get_element(const std::string& name)
{
  auto it = m_element_storage.find(name);

  if(it == m_element_storage.end())
  {
    return std::shared_ptr<GST_element_base>();
  }

  return it->second;
}

bool Gstreamer_pipeline::make_brio_pipeline()
{
  const std::string& cam_name  = m_camera_config.name;
  const std::string& pipe_name = m_pipeline_config.name;

  std::shared_ptr<GST_element_base> m_camera = std::make_shared<V4L2_webcam_pipe>();

  std::shared_ptr<GST_element_base> m_jpgdec;
  std::shared_ptr<GST_element_base> m_h264;
  std::shared_ptr<GST_element_base> m_thumb;

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

  if( ! m_camera->init(cam_name.c_str()) )
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

  std::shared_ptr<GST_element_base> m_h264_interpipesink = std::make_shared<GST_interpipesink>();
  if( ! m_h264_interpipesink->init("h264_ipsink_0") )
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

  std::shared_ptr<GST_element_base> m_rtppay = std::make_shared<rtp_h264_pipe>();
  if( ! m_rtppay->init("rtp_0") )
  {
   SPDLOG_ERROR("Could not init m_rtp");
   return false;
  }

  std::shared_ptr<GST_element_base> m_rtpsink = std::make_shared<rtpsink_pipe>();
  if( ! m_rtpsink->init("udp_0") )
  {
   SPDLOG_ERROR("Could not init m_udp");
   return false;
  }

  //add elements to top level bin
  m_camera->add_to_bin(m_pipeline);
  m_jpgdec->add_to_bin(m_pipeline);
  // m_test_src.add_to_bin(m_pipeline);
  m_thumb->add_to_bin(m_pipeline);
  m_h264->add_to_bin(m_pipeline);
  m_h264_interpipesink->add_to_bin(m_pipeline);
  // m_mkv.add_to_bin(m_pipeline);
  // m_display.add_to_bin(m_pipeline);
  m_rtppay->add_to_bin(m_pipeline);
  m_rtpsink->add_to_bin(m_pipeline);

  //link pipeline
  m_camera->link_back(m_jpgdec->front());
  m_camera->link_back(m_thumb->front());

  // m_jpgdec->link_back(m_display.front());
  m_jpgdec->link_back(m_h264->front());

  // m_test_src.link_back(m_display.front());
  // m_test_src.link_back(m_h264->front());

  m_h264->link_back(m_rtppay->front());
  m_h264->link_back(m_h264_interpipesink->front());

  m_rtppay->link_back(m_rtpsink->front());
  #endif

  m_element_storage.emplace(cam_name, m_camera);
  m_element_storage.emplace("jpgdec_0", m_jpgdec);
  m_element_storage.emplace("thumb_0", m_thumb);
  m_element_storage.emplace("h264_0", m_h264);
  m_element_storage.emplace("h264_ipsink_0", m_h264_interpipesink);
  m_element_storage.emplace("rtp_0", m_rtppay);
  m_element_storage.emplace("udp_0", m_rtpsink);

  return true;
}

bool Gstreamer_pipeline::make_imx219_pipeline()
{
  const std::string& cam_name  = m_camera_config.name;
  const std::string& pipe_name = m_pipeline_config.name;

  std::shared_ptr<nvac_imx219_pipe> m_camera   = std::make_shared<nvac_imx219_pipe>();
  if( ! m_camera->init(cam_name.c_str()) )
  {
   SPDLOG_ERROR("Could not init camera");
   return false;
  }
  
  m_camera->set_sensor_id(m_camera_config.get("properties.sensor-id", 0));
  // if( ! m_camera->set_sensor_id(m_camera_config.get("sensor-id", 0)))
  // {
  //     SPDLOG_ERROR("Could not set camera sensor-id");
  //     return false; 
  // }

<<<<<<< HEAD
  std::shared_ptr<GST_element_base> m_nvvideoconvert_pipe_t0   = std::make_shared<nvvideoconvert_pipe>();
  if( ! m_nvvideoconvert_pipe_t0->init("m_nvvideoconvert_pipe_t0") )
  {
    SPDLOG_ERROR("Could not init m_nvvideoconvert_pipe_t0");
    return false;
  }

  std::shared_ptr<GST_element_base> m_nvvideoconvert_pipe_t1   = std::make_shared<nvvideoconvert_pipe>();
  if( ! m_nvvideoconvert_pipe_t1->init("m_nvvideoconvert_pipe_t1") )
  {
    SPDLOG_ERROR("Could not init m_nvvideoconvert_pipe_t1");
    return false;
  }

  std::shared_ptr<timecodestamper> m_timecodestamper = std::make_shared<timecodestamper>();
  if( ! m_timecodestamper->init("timecodestamper_0") )
  {
    SPDLOG_ERROR("Could not init timecodestamper_0");
    return false;
  }

  std::shared_ptr<timeoverlay> m_timeoverlay = std::make_shared<timeoverlay>();
  if( ! m_timeoverlay->init("timeoverlay_0") )
  {
    SPDLOG_ERROR("Could not init timeoverlay_0");
    return false;
  }

  SPDLOG_INFO("NV mode");
  std::shared_ptr<GST_element_base> m_h264   = std::make_shared<h264_nvenc_bin>();
=======
>>>>>>> master
  std::shared_ptr<GST_element_base> m_thumb  = std::make_shared<Thumbnail_nv3_pipe>();
  if( ! m_thumb->init("thumb_0") )
  {
   SPDLOG_ERROR("Could not init thumb");
   return false;
  }

  std::shared_ptr<GST_element_base> m_h264   = std::make_shared<h264_nvenc_bin>();
  if( ! m_h264->init("h264_0") )
  {
   SPDLOG_ERROR("Could not init h264");
   return false;
  }

  std::shared_ptr<GST_element_base> m_h264_interpipesink = std::make_shared<GST_interpipesink>();
  if( ! m_h264_interpipesink->init("h264_ipsink_0") )
  {
   SPDLOG_ERROR("Could not init h264 interpipe");
   return false;
  }
  
  std::shared_ptr<GST_element_base> m_rtppay = std::make_shared<rtp_h264_pipe>();
  if( ! m_rtppay->init("rtp_0") )
  {
   SPDLOG_ERROR("Could not init m_rtp");
   return false;
  }
  
  std::shared_ptr<GST_element_base> m_rtpsink = std::make_shared<rtpsink_pipe>();
  if( ! m_rtpsink->init("udp_0") )
  {
   SPDLOG_ERROR("Could not init m_udp");
   return false;
  }

  //add elements to top level bin
  m_camera->add_to_bin(m_pipeline);
  
  m_nvvideoconvert_pipe_t0->add_to_bin(m_pipeline);
  m_timecodestamper->add_to_bin(m_pipeline);
  m_timeoverlay->add_to_bin(m_pipeline);
  m_nvvideoconvert_pipe_t1->add_to_bin(m_pipeline);

  m_thumb->add_to_bin(m_pipeline);
  m_h264->add_to_bin(m_pipeline);
  m_h264_interpipesink->add_to_bin(m_pipeline);
  m_rtppay->add_to_bin(m_pipeline);
  m_rtpsink->add_to_bin(m_pipeline);

  //link pipeline
  m_camera->link_back(m_thumb->front());

  // m_camera->link_back(m_h264->front());
  m_camera->link_back(m_nvvideoconvert_pipe_t0->front());
  m_nvvideoconvert_pipe_t0->link_back(m_timecodestamper->front());
  m_timecodestamper->link_back(m_timeoverlay->front());
  m_timeoverlay->link_back(m_nvvideoconvert_pipe_t1->front());
  m_nvvideoconvert_pipe_t1->link_back(m_h264->front());

  m_h264->link_back(m_rtppay->front());
  m_h264->link_back(m_h264_interpipesink->front());

  m_rtppay->link_back(m_rtpsink->front());

<<<<<<< HEAD
  m_element_storage.emplace("cam_0", m_camera);
  m_element_storage.emplace("timecodestamper_0", m_timecodestamper);
  m_element_storage.emplace("timeoverlay_0", m_timeoverlay);
=======
  m_element_storage.emplace(cam_name, m_camera);
>>>>>>> master
  m_element_storage.emplace("thumb_0", m_thumb);
  m_element_storage.emplace("h264_0", m_h264);
  m_element_storage.emplace("h264_ipsink_0", m_h264_interpipesink);
  m_element_storage.emplace("rtp_0", m_rtppay);
  m_element_storage.emplace("udp_0", m_rtpsink);

  return true;
}

bool Gstreamer_pipeline::make_alvium_pipeline()
{
  const std::string& cam_name  = m_camera_config.name;
  const std::string& pipe_name = m_pipeline_config.name;

  std::string format = m_camera_config.get<std::string>("properties.format");
  if(format.size() != 4)
  {
    SPDLOG_ERROR("fourcc format invalid: {:s}", format);
    return false;    
  }

  std::string device = m_camera_config.get<std::string>("properties.device");

  std::string trigger_mode = m_camera_config.get<std::string>("properties.trigger_mode");

  std::shared_ptr<V4L2_alvium_pipe> m_camera   = std::make_shared<V4L2_alvium_pipe>();
  m_camera->set_params(device.c_str(), v4l2_fourcc(format[0], format[1], format[2], format[3]), trigger_mode);
  if( ! m_camera->init(cam_name.c_str()) )
  {
    SPDLOG_ERROR("Could not init camera");
    return false;
  }

  std::shared_ptr<GST_element_base> m_thumb  = std::make_shared<Thumbnail_nv3_pipe>();
  if( ! m_thumb->init("thumb_0") )
  {
   SPDLOG_ERROR("Could not init thumb");
   return false;
  }

  std::shared_ptr<GST_element_base> m_h264   = std::make_shared<h264_nvenc_bin>();
  if( ! m_h264->init("h264_0") )
  {
   SPDLOG_ERROR("Could not init h264");
   return false;
  }

  std::shared_ptr<GST_element_base> m_h264_interpipesink = std::make_shared<GST_interpipesink>();
  if( ! m_h264_interpipesink->init("h264_ipsink_0") )
  {
   SPDLOG_ERROR("Could not init h264 interpipe");
   return false;
  }
  
  std::shared_ptr<GST_element_base> m_rtppay = std::make_shared<rtp_h264_pipe>();
  if( ! m_rtppay->init("rtp_0") )
  {
   SPDLOG_ERROR("Could not init m_rtp");
   return false;
  }
  
  std::shared_ptr<GST_element_base> m_rtpsink = std::make_shared<rtpsink_pipe>();
  if( ! m_rtpsink->init("udp_0") )
  {
   SPDLOG_ERROR("Could not init m_udp");
   return false;
  }

  //add elements to top level bin
  m_camera->add_to_bin(m_pipeline);
  m_thumb->add_to_bin(m_pipeline);
  m_h264->add_to_bin(m_pipeline);
  m_h264_interpipesink->add_to_bin(m_pipeline);
  m_rtppay->add_to_bin(m_pipeline);
  m_rtpsink->add_to_bin(m_pipeline);

  // //link pipeline
  if( ! m_camera->link_back(m_thumb->front()) )
  {
    SPDLOG_ERROR("Link failure");
    return false;
  }
  m_camera->link_back(m_h264->front());

  m_h264->link_back(m_rtppay->front());
  m_h264->link_back(m_h264_interpipesink->front());

  m_rtppay->link_back(m_rtpsink->front());

  m_element_storage.emplace(cam_name, m_camera);
  m_element_storage.emplace("thumb_0", m_thumb);
  m_element_storage.emplace("h264_0", m_h264);
  m_element_storage.emplace("h264_ipsink_0", m_h264_interpipesink);
  m_element_storage.emplace("rtp_0", m_rtppay);
  m_element_storage.emplace("udp_0", m_rtpsink);

  return true;
}

bool Gstreamer_pipeline::make_virtual_pipeline()
{
  const std::string& cam_name  = m_camera_config.name;
  const std::string& pipe_name = m_pipeline_config.name;

  std::shared_ptr<GST_element_base> m_camera = std::make_shared<Testsrc_pipe>();

  std::shared_ptr<GST_element_base> m_jpgdec;
  std::shared_ptr<GST_element_base> m_h264;
  std::shared_ptr<GST_element_base> m_thumb;

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

  if( ! m_camera->init(cam_name.c_str()) )
  {
   SPDLOG_ERROR("Could not init camera");
   return false;
  }


  std::shared_ptr<timecodestamper> m_timecodestamper = std::make_shared<timecodestamper>();
  if( ! m_timecodestamper->init("timecodestamper_0") )
  {
    SPDLOG_ERROR("Could not init timecodestamper_0");
    return false;
  }

  std::shared_ptr<timeoverlay> m_timeoverlay = std::make_shared<timeoverlay>();
  if( ! m_timeoverlay->init("timeoverlay_0") )
  {
    SPDLOG_ERROR("Could not init timeoverlay_0");
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

  std::shared_ptr<GST_element_base> m_h264_interpipesink = std::make_shared<GST_interpipesink>();
  if( ! m_h264_interpipesink->init("h264_ipsink_0") )
  {
   SPDLOG_ERROR("Could not init h264 interpipe");
   return false;
  }
  
  std::shared_ptr<GST_element_base> m_rtppay = std::make_shared<rtp_h264_pipe>();
  if( ! m_rtppay->init("rtp_0") )
  {
   SPDLOG_ERROR("Could not init m_rtp");
   return false;
  }
  
  std::shared_ptr<GST_element_base> m_rtpsink = std::make_shared<rtpsink_pipe>();
  if( ! m_rtpsink->init("udp_0") )
  {
   SPDLOG_ERROR("Could not init m_udp");
   return false;
  }

  //add elements to top level bin
  m_camera->add_to_bin(m_pipeline);

  m_timecodestamper->add_to_bin(m_pipeline);
  m_timeoverlay->add_to_bin(m_pipeline);

  m_thumb->add_to_bin(m_pipeline);
  m_h264->add_to_bin(m_pipeline);
  m_h264_interpipesink->add_to_bin(m_pipeline);

  m_rtppay->add_to_bin(m_pipeline);
  m_rtpsink->add_to_bin(m_pipeline);

  //link pipeline
  m_camera->link_back(m_thumb->front());

  m_camera->link_back(m_timecodestamper->front());
  m_timecodestamper->link_back(m_timeoverlay->front());
  m_timeoverlay->link_back(m_h264->front());

  m_h264->link_back(m_rtppay->front());
  m_h264->link_back(m_h264_interpipesink->front());

  m_rtppay->link_back(m_rtpsink->front());
  #endif

  m_element_storage.emplace(cam_name, m_camera);
  m_element_storage.emplace("thumb_0", m_thumb);
  m_element_storage.emplace("h264_0", m_h264);
  m_element_storage.emplace("h264_ipsink_0", m_h264_interpipesink);
  m_element_storage.emplace("rtp_0", m_rtppay);
  m_element_storage.emplace("udp_0", m_rtpsink);
  m_element_storage.emplace("timecodestamper_0", m_timecodestamper);
  m_element_storage.emplace("timeoverlay_0", m_timeoverlay);

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
  if( ! m_pipelines.empty() )
  {
    SPDLOG_INFO("Opaleye_app::init piplines already loaded");
    return false;
  }

  if(!m_config)
  {
    m_config = std::make_shared<app_config>();
    m_config->make_default();
  }

#if 0
  m_master_clock = std::make_shared<sys_clock>();
  if( ! m_master_clock->init() )
  {
    SPDLOG_ERROR("Opaleye_app::init master clock init failed");
    return false;
  }
  Gst::ClockTime base_time = m_master_clock->get_time();
#else
  bool ptp_sync_ok = false;
  for(int i = 0; i < 60; i++)
  {
    bool sync = m_master_clock->wait_for_sync();
    if( ! sync )
    {
      SPDLOG_WARN("Opaleye_app::init ptp clock sync failed");
    }
    else
    {
      ptp_sync_ok = true;
      SPDLOG_INFO("Opaleye_app::init ptp clock sync ready");
    }
  }

  if( ! ptp_sync_ok )
  {
    SPDLOG_ERROR("Opaleye_app::init ptp clock sync timed out");
    return false;
  }
#endif


  if(m_config->has_child("config.hw_trigger"))
  {
    if(! Opaleye_gpio_mod_ctrl::is_loaded() )
    {
      SPDLOG_ERROR("Opaleye_app::init could not open hw_trigger - module is not loaded");
      return false;
    }

    if( ! m_hw_trigger.open() )
    {
      SPDLOG_ERROR("Opaleye_app::init could not open hw_trigger");
      return false;
    }

    if( ! m_hw_trigger.disable() )
    {
      SPDLOG_ERROR("Opaleye_app::init could not disable hw_trigger");
      return false;
    }

    std::chrono::microseconds t0     = std::chrono::microseconds(m_config->get<int>("config.hw_trigger.t0",           0));
    std::chrono::microseconds period = std::chrono::microseconds(m_config->get<int>("config.hw_trigger.period", 1000000));
    std::chrono::microseconds width  = std::chrono::microseconds(m_config->get<int>("config.hw_trigger.width",    50000));
    if( ! m_hw_trigger.configure(t0, period, width) )
    {
      SPDLOG_ERROR("Opaleye_app::init could not configure hw_trigger");
      return false;
    }

    if( ! m_hw_trigger.set_enable(m_config->get<bool>("config.hw_trigger.enable", false)) )
    {
      SPDLOG_ERROR("Opaleye_app::init could not enable hw_trigger");
      return false;
    }
  }
  else
  {
    SPDLOG_ERROR("Opaleye_app::init not requested to init hw_trigger");
  }

  if(m_config->camera_configs.count("cam0"))
  {
    std::shared_ptr<Gstreamer_pipeline> pipeline = std::make_shared<Gstreamer_pipeline>();
    if( ! pipeline->init() )
    {
      SPDLOG_ERROR("Opaleye_app::init init pipe0 failed");
      return false;
    }

    {
      pipeline->use_clock(m_master_clock->get_clock());
      Glib::RefPtr<Gst::Element> pipe_elem = pipeline->get_pipeline();
      pipeline->get_pipeline()->set_start_time(GST_CLOCK_TIME_NONE);
      pipeline->get_pipeline()->set_latency(500 * GST_MSECOND);
      pipeline->get_pipeline()->set_base_time(base_time);
    }

    const camera_config& cam0_cfg     = m_config->camera_configs["cam0"];
    const std::string& cam0_cam_name  = cam0_cfg.name;
    const std::string& cam0_pipe_name = cam0_cfg.pipeline.name;

    if( ! pipeline->make_pipeline(m_config, cam0_cfg, cam0_cfg.pipeline) )
    {
      SPDLOG_ERROR("Opaleye_app::init make pipe0 failed");
      return false;
    }
    
    SPDLOG_INFO("Opaleye_app::init stashing cam0 pipeline: {:s}", cam0_pipe_name);
    m_pipelines.emplace(cam0_pipe_name, pipeline);
  }

  if(m_config->camera_configs.count("cam1"))
  {
    std::shared_ptr<Gstreamer_pipeline> pipeline = std::make_shared<Gstreamer_pipeline>();
    if( ! pipeline->init() )
    {
      SPDLOG_ERROR("Opaleye_app::init init pipe1 failed");
      return false;
    }

    {
      pipeline->use_clock(m_master_clock->get_clock());
      Glib::RefPtr<Gst::Element> pipe_elem = pipeline->get_pipeline();
      pipeline->get_pipeline()->set_start_time(GST_CLOCK_TIME_NONE);
      pipeline->get_pipeline()->set_latency(500 * GST_MSECOND);
      pipeline->get_pipeline()->set_base_time(base_time);
    }

    const camera_config& cam1_cfg     = m_config->camera_configs["cam1"];
    const std::string& cam1_cam_name  = cam1_cfg.name;
    const std::string& cam1_pipe_name = cam1_cfg.pipeline.name;

    if( ! pipeline->make_pipeline(m_config, cam1_cfg, cam1_cfg.pipeline) )
    {
      SPDLOG_ERROR("Opaleye_app::init make pipe1 failed");
      return false;
    }
    
    SPDLOG_INFO("Opaleye_app::init stashing cam1 pipeline: {:s}", cam1_pipe_name);
    m_pipelines.emplace(cam1_pipe_name, pipeline);
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

  auto pipeline_it = m_pipelines.find(camera);
  if(pipeline_it == m_pipelines.end())
  {
    SPDLOG_WARN("Opaleye_app::start_video_capture({:s}), {:s} does not exist", camera, camera);
    return false;
  }

  std::string cap_pipe_name = camera + "/file0";
  auto it = m_pipelines.find(cap_pipe_name);
  if(it != m_pipelines.end())
  {
    SPDLOG_WARN("Opaleye_app::start_video_capture({:s}), {:s} already exists", camera, cap_pipe_name);
    return false;
  }

  boost::filesystem::path out_video_dir = m_config->video_path / camera;
  
  if(boost::filesystem::exists(out_video_dir))
  {
    if( ! boost::filesystem::is_directory(out_video_dir) )
    {
      SPDLOG_WARN("Opaleye_app::start_video_capture({:s}), output destination {:s} exists but is not directory", camera, out_video_dir.string());
      return false;
    }
  }
  else
  {
    boost::system::error_code ec;
    if( ! boost::filesystem::create_directories(out_video_dir, ec) )
    {
      SPDLOG_WARN("Opaleye_app::start_video_capture({:s}), failed to create output directory {:s}: {:s}", camera, out_video_dir.string(), ec.message());
      return false;
    }
  }


  std::shared_ptr<gst_filesink_pipeline> m_mkv_pipe = std::make_shared<gst_filesink_pipeline>();
  m_mkv_pipe->set_top_storage_dir(out_video_dir.string());
  if(! m_mkv_pipe->init() )
  {
    SPDLOG_ERROR("m_mkv_pipe init failed");
    m_mkv_pipe.reset();
    return false;
  }

  m_mkv_pipe->set_listen_to("h264_ipsink_0");
  if(! m_mkv_pipe->start() )
  {
    SPDLOG_ERROR("m_mkv_pipe start failed");

    m_mkv_pipe.reset();
    return false;   
  }

  m_pipelines.emplace(cap_pipe_name, m_mkv_pipe);

  return true;
}
bool Opaleye_app::stop_video_capture(const std::string& camera)
{
  SPDLOG_INFO("Opaleye_app::stop_video_capture({:s})", camera);

  auto pipeline_it = m_pipelines.find(camera);
  if(pipeline_it == m_pipelines.end())
  {
    SPDLOG_WARN("Opaleye_app::stop_video_capture({:s}), {:s} does not exist", camera, camera);
    return false;
  }

  std::string cap_pipe_name = camera + "/file0";

  auto it = m_pipelines.find(cap_pipe_name);
  if( it == m_pipelines.end() )
  {
    SPDLOG_WARN("Opaleye_app::stop_video_capture({:s}), {:s} does not exist", camera, cap_pipe_name);
    return false;
  }

  if( ! it->second )
  {
    SPDLOG_ERROR("Opaleye_app::stop_video_capture({:s}), {:s} is found but is null", camera, cap_pipe_name);
    return false;
  }

  std::shared_ptr<gst_filesink_pipeline> m_mkv_pipe = std::dynamic_pointer_cast<gst_filesink_pipeline>(it->second);
  if( ! m_mkv_pipe )
  {
    throw jsonrpc::Fault("Could not downcast element", jsonrpc::Fault::INTERNAL_ERROR);
  }

  m_mkv_pipe->set_listen_to(NULL);
  m_mkv_pipe->send_pipeline_eos();
  m_mkv_pipe->wait_pipeline_eos();
  m_mkv_pipe->stop();
  m_mkv_pipe.reset();

  m_pipelines.erase(cap_pipe_name);

  return true;
}

bool Opaleye_app::start_still_capture(const std::string& camera)
{
  SPDLOG_INFO("Opaleye_app::start_still_capture({:s})", camera);

  throw jsonrpc::Fault("start_still_capture - not implemented", jsonrpc::Fault::INTERNAL_ERROR);

  return true;
}
bool Opaleye_app::stop_still_capture(const std::string& camera)
{
  SPDLOG_INFO("Opaleye_app::stop_still_capture({:s})", camera);

  throw jsonrpc::Fault("stop_still_capture - not implemented", jsonrpc::Fault::INTERNAL_ERROR);

  return true;
}

<<<<<<< HEAD
bool Opaleye_app::start_rtp_stream(const std::string& cam, const std::string& ip_addr, int port)
=======
bool Opaleye_app::start_rtp_stream(const std::string& pipe_name, const std::string& ip_addr, int port)
>>>>>>> master
{
  SPDLOG_INFO("Opaleye_app::start_rtp_stream {:s}:{:d}", ip_addr, port);

  if( (port < 0) || (port > 65535))
  {
    throw std::domain_error("port must be in [0, 65535]");
  }

<<<<<<< HEAD
  auto cam_pipe = m_pipelines.find(cam);
  if(cam_pipe == m_pipelines.end())
  {
    throw std::domain_error("Could not find cam"); 
  }

  std::shared_ptr<rtpsink_pipe> m_rtpsink = cam_pipe->second->get_element<rtpsink_pipe>("udp_0");
=======
  auto pipe_it = m_pipelines.find(pipe_name);
  if( pipe_it == m_pipelines.end() )
  {
    throw std::domain_error("pipeline does not exist");
  }

  std::shared_ptr<rtpsink_pipe> m_rtpsink = pipe_it->second->get_element<rtpsink_pipe>("udp_0");
>>>>>>> master
  if( ! m_rtpsink )
  {
    throw jsonrpc::Fault("Could not downcast element", jsonrpc::Fault::INTERNAL_ERROR);
  }

  return m_rtpsink->add_udp_client(ip_addr, port);
}
<<<<<<< HEAD
bool Opaleye_app::stop_rtp_stream(const std::string& cam, const std::string& ip_addr, int port)
=======
bool Opaleye_app::stop_rtp_stream(const std::string& pipe_name, const std::string& ip_addr, int port)
>>>>>>> master
{
  SPDLOG_INFO("Opaleye_app::stop_rtp_stream {:s}:{:d}", ip_addr, port);

  if( (port < 0) || (port > 65535))
  {
    throw std::domain_error("port must be in [0, 65535]");
  }

<<<<<<< HEAD
  auto cam_pipe = m_pipelines.find(cam);
  if(cam_pipe == m_pipelines.end())
  {
    throw std::domain_error("Could not find cam"); 
  }

  std::shared_ptr<rtpsink_pipe> m_rtpsink = cam_pipe->second->get_element<rtpsink_pipe>("udp_0");
=======
  auto pipe_it = m_pipelines.find(pipe_name);
  if( pipe_it == m_pipelines.end() )
  {
    throw std::domain_error("pipeline does not exist");
  }

  std::shared_ptr<rtpsink_pipe> m_rtpsink = pipe_it->second->get_element<rtpsink_pipe>("udp_0");
>>>>>>> master
  if( ! m_rtpsink )
  {
    throw jsonrpc::Fault("Could not downcast element", jsonrpc::Fault::INTERNAL_ERROR);
  }

  return m_rtpsink->remove_udp_client(ip_addr, port);
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
    v.second.reset();
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
  
  for(const auto& pipe_it : m_pipelines)
  {
    std::string pipe_name = pipe_it.first;
    std::replace(pipe_name.begin(), pipe_name.end(), '/', '-');

    const std::string basename      = fmt::format("{:s}_pipeline", pipe_name);
    const std::string dot_filename  = fmt::format("/tmp/{:s}.dot", basename);
    const std::string pdf_filename  = fmt::format("/tmp/{:s}.pdf", basename);

    pipe_it.second->make_debug_dot(basename);

    const std::string cmd_str  = fmt::format("dot -Tpdf -o {:s} {:s}", pdf_filename, dot_filename);  
    int ret = system(cmd_str.c_str());
    if(ret == -1)
    {
      SPDLOG_ERROR("Could not create pdf");
    }
  }

  // m_pipelines["cam0"]->make_debug_dot("pipeline");

  // auto it = m_pipelines.find("cam0/file0");
  // if(it == m_pipelines.end())
  // {
  //   return std::string();
  // }

  // if( ! it->second )
  // {
  //   return std::string();
  // }

  // std::shared_ptr<gst_filesink_pipeline> m_mkv_pipe = std::dynamic_pointer_cast<gst_filesink_pipeline>(it->second);

  // if(m_mkv_pipe)
  // {
  //   m_mkv_pipe->make_debug_dot("pipeline_mkv");
  //   int ret = system("dot -Tpdf -o /tmp/pipeline_mkv.dot.pdf /tmp/pipeline_mkv.dot");
  //   if(ret == -1)
  //   {
  //     SPDLOG_ERROR("Could not create pdf");
  //   }
  // }

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

bool Opaleye_app::set_camera_property_int(const std::string& pipeline_id, const std::string& camera_id, const std::string& property_id, int value)
{
  auto pipe_it = m_pipelines.find(pipeline_id);
  if(pipe_it == m_pipelines.end())
  {
    SPDLOG_ERROR("Could not get pipeline {:s}", pipeline_id);
    return false;
  }

  std::shared_ptr<GST_v4l2_api> cam = pipe_it->second->get_element<GST_v4l2_api>(camera_id);
  if( ! cam )
  {
    SPDLOG_ERROR("Could not get camera {:s}, is it a GST_v4l2_api", camera_id);
    return false;
  }

  bool ret = false;
  if(property_id == "brightness")
  {
    ret = cam->set_brightness(value);
  }
  else if(property_id == "gain")
  {
    ret = cam->set_gain(value);
  }
  else if(property_id == "gain_auto")
  {
    ret = cam->set_gain_auto(value);
  }
  else if(property_id == "exposure_auto")
  {
    ret = cam->set_exposure_auto(value);
  }
  else if(property_id == "exposure_absolute")
  {
    ret = cam->set_exposure_absolute(value);
  }
  else if(property_id == "exposure_auto_min")
  {
    ret = cam->set_exposure_auto_min(value);
  }
  else if(property_id == "exposure_auto_max")
  {
    ret = cam->set_exposure_auto_max(value);
  }
  else if(property_id == "gain_auto_min")
  {
    ret = cam->set_gain_auto_min(value);
  }
  else if(property_id == "gain_auto_max")
  {
    ret = cam->set_gain_auto_max(value);
  }
  else if(property_id == "focus_absolute")
  {
    ret = cam->set_focus_absolute(value);
  }
  else if(property_id == "focus_auto")
  {
    ret = cam->set_focus_auto(value);
  }
  else
  {
    ret = false; 
  }

  return ret;
}

bool Opaleye_app::set_camera_property_str(const std::string& pipeline_id, const std::string& camera_id, const std::string& property_id, const std::string& value)
{
  std::shared_ptr<GST_camera_base> cam;

  auto pipe_it = m_pipelines.find(pipeline_id);
  if(pipe_it == m_pipelines.end())
  {
    SPDLOG_ERROR("Could not get pipeline {:s}", pipeline_id);
    return false;
  }

  cam = pipe_it->second->get_element<GST_camera_base>(camera_id);
  if( ! cam )
  {
    SPDLOG_ERROR("Could not get camera {:s}, is it a GST_camera_base", camera_id);
    return false;
  }

  return cam->set_camera_property(property_id, value);
}