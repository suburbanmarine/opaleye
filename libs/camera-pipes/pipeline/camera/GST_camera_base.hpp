#pragma once

#include "pipeline/GST_element_base.hpp"

class GST_camera_base : public GST_element_base
{
public:

	GST_camera_base()
	{

	}
	~GST_camera_base() override
	{

	}

  typedef std::function<void(const std::string& metadata, const std::shared_ptr<const std::vector<uint8_t>>&)> FramebufferCallback;
  virtual void set_framebuffer_callback(const FramebufferCallback& cb) = 0;
};
