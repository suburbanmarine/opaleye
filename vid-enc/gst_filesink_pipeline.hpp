#include "pipeline/GST_app_base.hpp"
#include "pipeline/GST_interpipesrc.hpp"

#include "pipeline/storage/mkv_multifilesink_pipe.hpp"

class gst_filesink_pipeline : public GST_app_base
{
public:

  gst_filesink_pipeline();

  ~gst_filesink_pipeline() override;

  bool init() override;

  void set_listen_to(const char name[]);

  void send_pipeline_eos();
  void wait_pipeline_eos();

protected:

	GST_interpipesrc       m_interpipe_src;
	mkv_multifilesink_pipe m_mkv;
};