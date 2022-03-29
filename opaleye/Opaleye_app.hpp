/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "gst_filesink_pipeline.hpp"

#include "config/Opaleye_config_mgr.hpp"

#include "pipeline/GST_app_base.hpp"
#include "pipeline/GST_interpipesink.hpp"

#include "pipeline/camera/Testsrc_pipe.hpp"
// #include "pipeline/camera/Logitech_brio_pipe.hpp"
#include "pipeline/camera/V4L2_webcam_pipe.hpp"
#include "pipeline/camera/nvac_imx219_pipe.hpp"
#include "pipeline/display/autovideosink_pipe.hpp"
#include "pipeline/stream/rtp_h264_pipe.hpp"
#include "pipeline/stream/rtpsink_pipe.hpp"
#include "pipeline/Thumbnail_sw_pipe.hpp"

#include <memory>

// REST URL endpoints

// Get sensor data
// /api/v1/sensors/
// /api/v1/sensors/pressure
// /api/v1/sensors/pressure/0
// /api/v1/sensors/temperature
// /api/v1/sensors/temperature/0
// /api/v1/sensors/temperature/1

// Get camera info
// /api/v1/cameras/
// /api/v1/cameras/<camera-name>/properties
// /api/v1/cameras/<camera-name>/live/full
// /api/v1/cameras/<camera-name>/live/preview

// Get camera RTP stream
// /api/v1/cameras/
// /api/v1/cameras/<camera-name>/rtp
// /api/v1/cameras/<camera-name>/rtp/clients

// Get pipeline info
// /api/v1/cameras/<camera-name>/pipelines
// /api/v1/cameras/<camera-name>/pipelines/<pipeline-name>/properties
// /api/v1/cameras/<camera-name>/pipelines/<pipeline-name>/graph

// Get config file
// /api/v1/config

// /api/v1/system/
// /api/v1/system/nvpmodel/
// /api/v1/system/fan/

// /api/v1/util/
// /api/v1/util/time

// Other RPC calls
// /api/v1/rpc

class Gstreamer_pipeline : public GST_app_base
{
public:

  Gstreamer_pipeline();

  ~Gstreamer_pipeline() override;

  bool init() override;
  bool make_pipeline(const std::shared_ptr<const app_config>& app_config, const camera_config& camera_config, const pipeline_config& pipe_config);

  bool has_element(const std::string& name);

  std::shared_ptr<GST_element_base> get_element(const std::string& name) override;

  template< typename T >
  bool add_element(const std::string& name, const std::shared_ptr<GST_element_base>& node)
  {
    // if( m_element_storage.contains(name) )
    // {
    //   return false;
    // }

    if( m_element_storage.find(name) != m_element_storage.end())
    {
      return false;
    }

    m_element_storage.emplace(name, node);
    return true;
  }

  //todo - this should be registered at Opaleye_app::m_pipelines, not here
  // std::shared_ptr<gst_filesink_pipeline> m_mkv_pipe;

protected:
    
  bool make_brio_pipeline();
  bool make_imx219_pipeline();
  bool make_virtual_pipeline();


  std::map<std::string, std::shared_ptr<GST_element_base>> m_element_storage;

  camera_config                     m_camera_config;
  pipeline_config                   m_pipeline_config;
  std::shared_ptr<const app_config> m_app_config;
};

class Opaleye_app
{
public:

  Opaleye_app();

  ~Opaleye_app();

  bool init();

  //The API Handlers
  //May be called by local code or the jsonrpc interface or the FCGI interface
  //Must be MT safe - multiple calls may be concurrent on different threads
  //May block - this will gate new client connection processing, this is configurable between NGINX and our FCGI settings
  std::vector<std::string> get_camera_list() const;
  bool start_camera(const std::string& camera);
  bool stop_camera(const std::string& camera);

  bool start_video_capture(const std::string& camera);
  bool stop_video_capture(const std::string& camera);

  bool start_still_capture(const std::string& camera);
  bool stop_still_capture(const std::string& camera);

  bool start_rtp_stream(const std::string& ip_addr, int port);
  bool stop_rtp_stream(const std::string& ip_addr, int port);
  bool stop_rtp_all_stream();
  std::string get_sdp_file() const;

  ///
  /// Start all pipelines.
  ///
  bool start();

  ///
  /// Stop all pipelines.
  ///
  bool stop();

  ///
  /// Null, running, paused, ...
  ///
  std::string get_pipeline_status();
  std::string get_pipeline_graph();

  /* config */
  void set_config(const std::string& config);
  std::string get_config() const;
  void set_default_config();

  /* power */
  void restart_software();
  void reboot();
  void shutdown();

  /* util */
  std::string get_time() const;
  bool set_time(const std::string& time) const;
  std::string get_mac_addr() const;

  /* storage */
  std::vector<std::string> get_storage_path_list() const;
  void format(const std::string& path) const;
  void erase(const std::string& path) const;
  uint64_t get_free_space(const std::string& path) const;

  /* health */
  void get_load() const;
  void get_cpu() const;

  /* debug / devel */
  ///
  /// Force a WD reset
  ///
  bool test_watchdog() const;

  ///
  /// on, off, blink
  ///
  bool set_user_led(const std::string& mode) const;

  /* info */
  std::string get_sw_version() const;
  std::string get_hw_version() const;
  std::string get_serial_number() const;

  /* cameras */
  bool set_camera_property(const std::string& camera_id, const std::string& property_id, int value);

// protected:

  //pipeline name to pipeline
  std::map<std::string, std::shared_ptr<GST_app_base>> m_pipelines;

  std::shared_ptr<app_config> m_config;

};
