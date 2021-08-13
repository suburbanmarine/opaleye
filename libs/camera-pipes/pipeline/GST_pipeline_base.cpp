/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "pipeline/GST_pipeline_base.hpp"

GST_pipeline_base::GST_pipeline_base(const char name[])
{
  m_pipe = Gst::Pipeline::create(name);
  m_bus = m_pipe->get_bus();
}

GST_pipeline_base::~GST_pipeline_base()
{
  
}