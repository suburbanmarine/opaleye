#pragma once

#include "pipeline/GST_app_base.hpp"
#include "pipeline/GST_interpipesrc.hpp"

#include "pipeline/storage/mp4_splitmuxsink.hpp"

class gst_filesink_pipeline : public GST_app_base
{
public:

  gst_filesink_pipeline();

  ~gst_filesink_pipeline() override;

  void set_top_storage_dir(const std::string& s)
  {
    top_storage_dir = s;
  }
  bool init() override;

  void set_listen_to(const char name[]);

  void send_pipeline_eos();
  void wait_pipeline_eos();

protected:

    //parameters
    boost::filesystem::path top_storage_dir;

	GST_interpipesrc m_interpipe_src;
	mp4_splitmuxsink m_mux;
};