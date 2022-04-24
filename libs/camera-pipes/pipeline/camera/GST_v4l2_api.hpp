#pragma once

#include "pipeline/camera/GST_camera_base.hpp"

#include "util/v4l2_util.hpp"

class GST_v4l2_api : public GST_camera_base
{
public:

	GST_v4l2_api()
	{

	}
	~GST_v4l2_api() override
	{

	}

//User Controls
  // virtual bool set_exposure(int32_t val);
  // virtual bool get_exposure(int32_t* const val);

  virtual bool set_brightness(int32_t val);
  virtual bool get_brightness(int32_t* const val);
  
  virtual bool set_gain(int32_t val);
  virtual bool get_gain(int32_t* const val);

  virtual bool set_gain_auto(bool val);
  virtual bool get_gain_auto(bool* const val);

//Camera Controls

  virtual bool set_exposure_auto(int32_t val);
  virtual bool get_exposure_auto(int32_t* const val);

  virtual bool set_exposure_absolute(int32_t val);
  virtual bool get_exposure_absolute(int32_t* const val);

  virtual bool set_exposure_auto_min(int32_t val);
  virtual bool get_exposure_auto_min(int32_t* const val);

  virtual bool set_exposure_auto_max(int32_t val);
  virtual bool get_exposure_auto_max(int32_t* const val);

  virtual bool set_gain_auto_min(int32_t val);
  virtual bool get_gain_auto_min(int32_t* const val);

  virtual bool set_gain_auto_max(int32_t val);
  virtual bool get_gain_auto_max(int32_t* const val);

  virtual bool set_focus_absolute(int32_t val);
  virtual bool get_focus_absolute(int32_t* const val);

  virtual bool set_focus_auto(bool val);
  virtual bool get_focus_auto(bool* const val);

protected:
    v4l2_util m_v4l2_util;
};
