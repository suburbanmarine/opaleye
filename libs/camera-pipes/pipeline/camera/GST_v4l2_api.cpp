#include "GST_v4l2_api.hpp"

bool GST_v4l2_api::set_exposure_mode(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_EXPOSURE_AUTO, val);
}
bool GST_v4l2_api::get_exposure_mode(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_EXPOSURE_AUTO, val);
}
bool GST_v4l2_api::set_exposure_value(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_EXPOSURE_ABSOLUTE, val);
}
bool GST_v4l2_api::get_exposure_value(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_EXPOSURE_ABSOLUTE, val);
}

bool GST_v4l2_api::set_focus_absolute(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_FOCUS_ABSOLUTE, val);
}
bool GST_v4l2_api::get_focus_absolute(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_FOCUS_ABSOLUTE, val);
}

bool GST_v4l2_api::set_focus_auto(bool val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_FOCUS_AUTO, val);
}
bool GST_v4l2_api::get_focus_auto(bool* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_FOCUS_AUTO, val);
}

bool GST_v4l2_api::set_brightness(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_BRIGHTNESS, val);
}
bool GST_v4l2_api::get_brightness(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_BRIGHTNESS, val);
}

bool GST_v4l2_api::set_gain(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_GAIN, val);
}
bool GST_v4l2_api::get_gain(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_GAIN, val);
}
