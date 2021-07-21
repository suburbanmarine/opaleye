#pragma once

#include "errno_util.hpp"

#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <cstdint>

class v4l2_util
{
public:

	v4l2_util();
	~v4l2_util();

	void set_fd(int fd)
	{
		m_v4l2_fd = fd;
	}

	bool v4l2_ctrl_set(uint32_t id, const bool val);
	bool v4l2_ctrl_set(uint32_t id, const int32_t val);
	bool v4l2_ctrl_set(uint32_t id, const int64_t val);
	bool v4l2_ctrl_set(uint32_t id, const uint8_t val);
	bool v4l2_ctrl_set(uint32_t id, const uint16_t val);
	bool v4l2_ctrl_set(uint32_t id, const uint32_t val);

	bool v4l2_ctrl_get(uint32_t id, bool*    const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_CUR_VAL, out_val);
	}
	bool v4l2_ctrl_get(uint32_t id, int32_t* const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_CUR_VAL, out_val);
	}
	bool v4l2_ctrl_get(uint32_t id, int64_t* const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_CUR_VAL, out_val);
	}
	bool v4l2_ctrl_get(uint32_t id, uint8_t* const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_CUR_VAL, out_val);
	}
	bool v4l2_ctrl_get(uint32_t id, uint16_t* const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_CUR_VAL, out_val);
	}
	bool v4l2_ctrl_get(uint32_t id, uint32_t* const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_CUR_VAL, out_val);
	}

	bool v4l2_ctrl_get_def(uint32_t id, bool*    const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_DEF_VAL, out_val);
	}
	bool v4l2_ctrl_get_def(uint32_t id, int32_t* const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_DEF_VAL, out_val);
	}
	bool v4l2_ctrl_get_def(uint32_t id, int64_t* const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_DEF_VAL, out_val);
	}
	bool v4l2_ctrl_get_def(uint32_t id, uint8_t* const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_DEF_VAL, out_val);
	}
	bool v4l2_ctrl_get_def(uint32_t id, uint16_t* const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_DEF_VAL, out_val);
	}
	bool v4l2_ctrl_get_def(uint32_t id, uint32_t* const out_val)
	{
		return v4l2_ctrl_get(id, V4L2_CTRL_WHICH_DEF_VAL, out_val);
	}

	bool v4l2_ctrl_get(uint32_t id, uint32_t which, bool*    const out_val);
	bool v4l2_ctrl_get(uint32_t id, uint32_t which, int32_t* const out_val);
	bool v4l2_ctrl_get(uint32_t id, uint32_t which, int64_t* const out_val);
	bool v4l2_ctrl_get(uint32_t id, uint32_t which, uint8_t* const out_val);
	bool v4l2_ctrl_get(uint32_t id, uint32_t which, uint16_t* const out_val);
	bool v4l2_ctrl_get(uint32_t id, uint32_t which, uint32_t* const out_val);

	bool v4l2_ctrl_set(v4l2_ext_control* const ctrl);
	bool v4l2_ctrl_get(uint32_t which, v4l2_ext_control* const ctrl);
protected:
	int m_v4l2_fd;

	errno_util m_errno;
};