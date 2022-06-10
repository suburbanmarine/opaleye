/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

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

  //todo refactor this so the node storage by name is in base GST_app_base
  std::shared_ptr<GST_element_base> get_element(const std::string& name) override
  {
    return std::shared_ptr<GST_element_base>();
  }

protected:

  //parameters
  boost::filesystem::path top_storage_dir;

	GST_interpipesrc m_interpipe_src;
	mp4_splitmuxsink m_mux;
};