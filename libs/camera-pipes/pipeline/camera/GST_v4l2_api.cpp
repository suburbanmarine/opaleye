#include "GST_v4l2_api.hpp"

//User Controls
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

bool GST_v4l2_api::set_gain_auto(bool val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_AUTOGAIN, val);
}
bool GST_v4l2_api::get_gain_auto(bool* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_AUTOGAIN, val);
}

//Camera Controls

bool GST_v4l2_api::set_exposure_auto(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_EXPOSURE_AUTO, val);
}
bool GST_v4l2_api::get_exposure_auto(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_EXPOSURE_AUTO, val);
}
bool GST_v4l2_api::set_exposure_absolute(int32_t val)
{
	return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_EXPOSURE_ABSOLUTE, val);
}
bool GST_v4l2_api::get_exposure_absolute(int32_t* const val)
{
	return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_EXPOSURE_ABSOLUTE, val);
}

bool GST_v4l2_api::set_exposure_auto_min(int32_t val)
{
	auto ctrl_id = m_v4l2_util.get_ctrl_id_by_name("exposure_auto_min");

	if( ! ctrl_id )
	{
		return false;
	}

	return m_v4l2_util.v4l2_ctrl_set(ctrl_id.value(), val);
}
bool GST_v4l2_api::get_exposure_auto_min(int32_t* const val)
{
	auto ctrl_id = m_v4l2_util.get_ctrl_id_by_name("exposure_auto_min");

	if( ! ctrl_id )
	{
		return false;
	}

	return m_v4l2_util.v4l2_ctrl_get(ctrl_id.value(), val);
}

bool GST_v4l2_api::set_exposure_auto_max(int32_t val)
{
	auto ctrl_id = m_v4l2_util.get_ctrl_id_by_name("exposure_auto_max");

	if( ! ctrl_id )
	{
		return false;
	}

	return m_v4l2_util.v4l2_ctrl_set(ctrl_id.value(), val);
}
bool GST_v4l2_api::get_exposure_auto_max(int32_t* const val)
{
	auto ctrl_id = m_v4l2_util.get_ctrl_id_by_name("exposure_auto_max");

	if( ! ctrl_id )
	{
		return false;
	}

	return m_v4l2_util.v4l2_ctrl_get(ctrl_id.value(), val);
}

bool GST_v4l2_api::set_gain_auto_min(int32_t val)
{
	auto ctrl_id = m_v4l2_util.get_ctrl_id_by_name("gain_auto_min");

	if( ! ctrl_id )
	{
		return false;
	}

	return m_v4l2_util.v4l2_ctrl_set(ctrl_id.value(), val);
}
bool GST_v4l2_api::get_gain_auto_min(int32_t* const val)
{
	auto ctrl_id = m_v4l2_util.get_ctrl_id_by_name("gain_auto_min");

	if( ! ctrl_id )
	{
		return false;
	}

	return m_v4l2_util.v4l2_ctrl_get(ctrl_id.value(), val);
}

bool GST_v4l2_api::set_gain_auto_max(int32_t val)
{
	auto ctrl_id = m_v4l2_util.get_ctrl_id_by_name("gain_auto_max");

	if( ! ctrl_id )
	{
		return false;
	}

	return m_v4l2_util.v4l2_ctrl_set(ctrl_id.value(), val);
}
bool GST_v4l2_api::get_gain_auto_max(int32_t* const val)
{
	auto ctrl_id = m_v4l2_util.get_ctrl_id_by_name("gain_auto_max");

	if( ! ctrl_id )
	{
		return false;
	}

	return m_v4l2_util.v4l2_ctrl_get(ctrl_id.value(), val);
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


