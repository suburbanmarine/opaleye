#include "GST_camera_base.hpp"

#include <stdexcept>

void GST_camera_base::set_framebuffer_callback(const FramebufferCallback& cb)
{
	throw std::runtime_error("Not implemented");
}

bool GST_camera_base::set_exposure_mode(int32_t val)
{
	return false;
}
bool GST_camera_base::get_exposure_mode(int32_t* const val)
{
	return false;
}

bool GST_camera_base::set_exposure_value(int32_t val)
{
	return false;
}
bool GST_camera_base::get_exposure_value(int32_t* const val)
{
	return false;
}

bool GST_camera_base::set_focus_absolute(int32_t val)
{
	return false;
}
bool GST_camera_base::get_focus_absolute(int32_t* const val)
{
	return false;
}

bool GST_camera_base::set_focus_auto(bool val)
{
	return false;
}
bool GST_camera_base::get_focus_auto(bool* const val)
{
	return false;
}

bool GST_camera_base::set_brightness(int32_t val)
{
	return false;
}
bool GST_camera_base::get_brightness(int32_t* const val)
{
	return false;
}

bool GST_camera_base::set_gain(int32_t val)
{
	return false;
}
bool GST_camera_base::get_gain(int32_t* const val)
{
	return false;
}

bool GST_camera_base::set_camera_property(const std::string& property_id, const std::string& value)
{
	return false;	
}
