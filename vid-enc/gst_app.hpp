#include "pipeline/GST_app_base.hpp"
#include "pipeline/camera/Logitech_brio_pipe.hpp"
#include "pipeline/storage/mkv_multifilesink_pipe.hpp"
#include "pipeline/display/autovideosink_pipe.hpp"
#include "pipeline/stream/rtp_udp_pipe.hpp"

#include <memory>

class test_app : public GST_app_base
{
public:

  test_app();

  ~test_app() override;

  bool init() override;

protected:
  Logitech_brio_pipe m_logi_brio;

  std::shared_ptr<GST_element_base> m_jpgdec;
  std::shared_ptr<GST_element_base> m_h264;

  mkv_multifilesink_pipe m_mkv;
  rtp_udp_pipe       m_rtp;
  autovideosink_pipe m_display;
};

