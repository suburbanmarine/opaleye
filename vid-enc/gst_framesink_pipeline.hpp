#include "pipeline/GST_app_base.hpp"
#include "pipeline/GST_interpipesrc.hpp"

#include "pipeline/storage/gst_framesink_pipeline.hpp"

class gst_framesink_pipeline : public GST_app_base
{
public:

  gst_framesink_pipeline();

  ~gst_framesink_pipeline() override;

  bool init() override;

  void set_listen_to(const char name[]);

  void send_pipeline_eos();
  void wait_pipeline_eos();

protected:

	GST_interpipesrc m_interpipe_src;
	framegrab_bin m_frame;
};