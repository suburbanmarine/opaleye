#include "gst_app.hpp"

#include "pipeline/encode/h264_swenc_pipe.hpp"
#include "pipeline/decode/jpeg_swdec_pipe.hpp"

#include "pipeline/encode/h264_nvenc_pipe.hpp"
#include "pipeline/decode/jpeg_nvdec_pipe.hpp"

#include <spdlog/spdlog.h>

test_app::test_app()
{

}

test_app::~test_app()
{

}

bool test_app::init()
{
  if ( ! GST_app_base::init() )
  {
    SPDLOG_ERROR("Could not init GST_app_base");
    return false;
  }

  if(true)
  {
    m_jpgdec = std::make_shared<jpeg_swdec_pipe>();
    m_h264 = std::make_shared<h264_swenc_pipe>();
  }
  else
  {
    // https://forums.developer.nvidia.com/t/bus-error-with-gstreamer-and-opencv/110657/5
    // libjpeg and nvjpegdec may not be used in the same program...
    // m_jpgdec = std::make_shared<jpeg_nvdec_pipe>();
    m_jpgdec = std::make_shared<jpeg_swdec_pipe>();
    m_h264 = std::make_shared<h264_nvenc_pipe>(); 
  }

  if( ! m_logi_brio.init("cam_0") )
  {
   SPDLOG_ERROR("Could not init camera");
   return false;
  }

  if( ! m_jpgdec->init("jpgdec_0") )
  {
   SPDLOG_ERROR("Could not init jpgdec");
   return false;
  }

  if( ! m_h264->init("h264_0") )
  {
   SPDLOG_ERROR("Could not init h264");
   return false;
  }

  if( ! m_mkv.init("mkv_0") )
  {
   SPDLOG_ERROR("Could not init mkv");
   return false;
  }

  if( ! m_display.init("display_0") )
  {
   SPDLOG_ERROR("Could not init m_display");
   return false;
  }

  if( ! m_rtp.init("rtp_0") )
  {
   SPDLOG_ERROR("Could not init m_display");
   return false;
  }

  //add elements to top level bin
  m_logi_brio.add_to_bin(m_pipeline);
  m_jpgdec->add_to_bin(m_pipeline);
  m_h264->add_to_bin(m_pipeline);
  m_mkv.add_to_bin(m_pipeline);
  m_display.add_to_bin(m_pipeline);
  // m_rtp.add_to_bin(m_pipeline);

  //link pipeline
  m_logi_brio.link_back(m_jpgdec->front());

  m_jpgdec->link_back(m_display.front());
  m_jpgdec->link_back(m_h264->front());

  m_h264->link_back(m_mkv.front());
  // m_h264->link_back(m_rtp.front());
  

  return true;
}
