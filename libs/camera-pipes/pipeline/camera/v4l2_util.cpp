#include "v4l2_util.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

#include <sys/ioctl.h>

#include <cstring>

#include <array>

v4l2_util::v4l2_util()
{
	m_v4l2_fd = -1;
}
v4l2_util::~v4l2_util()
{

}

bool v4l2_util::v4l2_ctrl_set(uint32_t id, const bool val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;
	ctrl.value = (val) ? 1 : 0;

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}

bool v4l2_util::v4l2_ctrl_set(uint32_t id, const int32_t val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;
	ctrl.value = val;

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}

bool v4l2_util::v4l2_ctrl_set(uint32_t id, const int64_t val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;
	ctrl.value64 = val;

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}

bool v4l2_util::v4l2_ctrl_set(uint32_t id, const uint8_t val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 1;

	std::array<uint8_t, 1> arr = {val};
	ctrl.p_u8  = arr.data();

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;	
}
bool v4l2_util::v4l2_ctrl_set(uint32_t id, const uint16_t val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 2;

	std::array<uint16_t, 1> arr = {val};
	ctrl.p_u16 = arr.data();

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}
bool v4l2_util::v4l2_ctrl_set(uint32_t id, const uint32_t val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 4;
	
	std::array<uint32_t, 1> arr = {val};
	ctrl.p_u32 = arr.data();

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}

bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, bool* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = (ctrl.value) ? true : false;
	return true;
}

bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, int32_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = ctrl.value;
	return true;
}

bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, int64_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = ctrl.value64;
	return true;
}

bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, uint8_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 1;

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = ctrl.p_u8[0];
	return true;
}
bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, uint16_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 2;

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = ctrl.p_u16[0];
	return true;
}
bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, uint32_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 4;

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = ctrl.p_u32[0];
	return true;
}

bool v4l2_util::v4l2_ctrl_set(v4l2_ext_control* const ctrl)
{
	v4l2_ext_controls ctrls;
	memset(&ctrls, 0, sizeof(ctrls));
	ctrls.which      = V4L2_CTRL_WHICH_CUR_VAL;
	ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(ctrl->id);
	ctrls.count      = 1;
	ctrls.controls   = ctrl;

	int ret = ioctl(m_v4l2_fd, VIDIOC_S_EXT_CTRLS, &ctrls);	
	if(ret < 0)
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}
bool v4l2_util::v4l2_ctrl_get(uint32_t which, v4l2_ext_control* const ctrl)
{
	v4l2_ext_controls ctrls;
	memset(&ctrls, 0, sizeof(ctrls));
	ctrls.which      = which;
	ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(ctrl->id);
	ctrls.count      = 1;
	ctrls.controls   = ctrl;

	int ret = ioctl(m_v4l2_fd, VIDIOC_G_EXT_CTRLS, &ctrls);	
	if(ret < 0)
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}