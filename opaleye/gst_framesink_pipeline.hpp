/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "pipeline/GST_app_base.hpp"
#include "pipeline/GST_interpipesrc.hpp"

#include "pipeline/storage/framegrab_bin.hpp"

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