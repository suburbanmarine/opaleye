/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

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
  virtual void set_framebuffer_callback(const FramebufferCallback& cb);

  virtual bool set_exposure_mode(int32_t val);
  virtual bool get_exposure_mode(int32_t* const val);

  virtual bool set_exposure_value(int32_t val);
  virtual bool get_exposure_value(int32_t* const val);

  virtual bool set_focus_absolute(int32_t val);
  virtual bool get_focus_absolute(int32_t* const val);

  virtual bool set_focus_auto(bool val);
  virtual bool get_focus_auto(bool* const val);

  virtual bool set_brightness(int32_t val);
  virtual bool get_brightness(int32_t* const val);

  virtual bool set_gain(int32_t val);
  virtual bool get_gain(int32_t* const val);

  virtual bool set_camera_property(const std::string& property_id, const std::string& value);
  virtual bool get_camera_property(const std::string& property_id, std::string* const out_value);
};
