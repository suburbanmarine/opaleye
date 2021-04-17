#include "pipeline/GST_app_base.hpp"
#include "pipeline/camera/Testsrc_pipe.hpp"
#include "pipeline/camera/Logitech_brio_pipe.hpp"
#include "pipeline/storage/mkv_multifilesink_pipe.hpp"
#include "pipeline/display/autovideosink_pipe.hpp"
#include "pipeline/stream/rtp_h264_pipe.hpp"
#include "pipeline/stream/rtpsink_pipe.hpp"
#include <memory>

class test_app_config
{
public:
  bool serialize();
  bool deserialize();
  void set_default();
protected:
  // boost::
};

class test_app : public GST_app_base
{
public:

  test_app();

  ~test_app() override;

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
  std::string get_pipeline_status() const;

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
  std::string get_sw_version() const;
  std::string get_hw_version() const;
  std::string get_serial_number() const;

// protected:
  Testsrc_pipe       m_test_src;
  Logitech_brio_pipe m_logi_brio;

  std::shared_ptr<GST_element_base> m_jpgdec;
  std::shared_ptr<GST_element_base> m_h264;

  mkv_multifilesink_pipe m_mkv;
  rtp_h264_pipe          m_rtppay;
  rtpsink_pipe           m_rtpsink;
  autovideosink_pipe     m_display;
};

