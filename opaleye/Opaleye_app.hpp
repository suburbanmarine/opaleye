/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "gst_filesink_pipeline.hpp"

#include "app_config.hpp"

#include "pipeline/GST_app_base.hpp"
#include "pipeline/GST_interpipesink.hpp"

#include "pipeline/camera/Testsrc_pipe.hpp"
// #include "pipeline/camera/Logitech_brio_pipe.hpp"
#include "pipeline/camera/v4l2_webcam_pipe.hpp"
#include "pipeline/display/autovideosink_pipe.hpp"
#include "pipeline/stream/rtp_h264_pipe.hpp"
#include "pipeline/stream/rtpsink_pipe.hpp"
#include "pipeline/Thumbnail_sw_pipe.hpp"

#include <memory>

class Opaleye_app : public GST_app_base
{
public:

  Opaleye_app();

  ~Opaleye_app() override;

  bool init() override;

  //The API Handlers
  //May be called by local code or the jsonrpc interface or the FCGI interface
  //Must be MT safe - multiple calls may be concurrent on different threads
  //May block
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
  /// Stop most activity.
  ///
  // bool stop();

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
  Testsrc_pipe       m_test_src;
  V4L2_webcam_pipe   m_camera;

  std::shared_ptr<GST_element_base> m_jpgdec;
  std::shared_ptr<GST_element_base> m_h264;

  std::shared_ptr<Thumbnail_pipe_base> m_thumb;

  GST_interpipesink      m_h264_interpipesink;

  rtp_h264_pipe          m_rtppay;
  rtpsink_pipe           m_rtpsink;
  autovideosink_pipe     m_display;

  std::shared_ptr<gst_filesink_pipeline> m_mkv_pipe;

  std::shared_ptr<app_config> m_config;

};

