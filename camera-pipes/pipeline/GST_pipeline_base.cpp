#include "pipeline/GST_pipeline_base.hpp"

GST_pipeline_base::GST_pipeline_base(const char name[])
{
  m_pipe = Gst::Pipeline::create(name);
  m_bus = m_pipe->get_bus();
}

GST_pipeline_base::~GST_pipeline_base()
{
  
}