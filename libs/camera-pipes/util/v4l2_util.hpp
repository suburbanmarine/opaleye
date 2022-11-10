/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "util/v4l2_ctrl.hpp"

#include "opaleye-util/errno_util.hpp"

#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <cstdint>

#include <list>
#include <vector>
#include <map>
#include <optional>

class v4l2_util
{
public:

	v4l2_util();
	~v4l2_util();

	static std::string fourcc_to_str(const uint32_t fcc);

	static const char* v4l2_field_to_str(const __u32& val)
	{
		return v4l2_field_to_str((v4l2_field)val);
	}
	static const char* v4l2_colorspace_to_str(const __u32& val)
	{
		return v4l2_colorspace_to_str((v4l2_colorspace)val);
	}
	static const char* v4l2_ycbcr_encoding_to_str(const __u32& val)
	{
		return v4l2_ycbcr_encoding_to_str((v4l2_ycbcr_encoding)val);
	}
	static const char* v4l2_hsv_encoding_to_str(const __u8& val)
	{
		return v4l2_hsv_encoding_to_str((v4l2_hsv_encoding)val);
	}
	static const char* v4l2_quantization_to_str(const __u8& val)
	{
		return v4l2_quantization_to_str((v4l2_quantization)val);
	}
	static const char* v4l2_xfer_func_to_str(const __u8& val)
	{
		return v4l2_xfer_func_to_str((v4l2_xfer_func)val);
	}

	static const char* v4l2_field_to_str(const v4l2_field& val);
	static const char* v4l2_colorspace_to_str(const v4l2_colorspace& val);
	static const char* v4l2_ycbcr_encoding_to_str(const v4l2_ycbcr_encoding& val);
	static const char* v4l2_hsv_encoding_to_str(const v4l2_hsv_encoding& val);
	static const char* v4l2_quantization_to_str(const v4l2_quantization& val);
	static const char* v4l2_xfer_func_to_str(const v4l2_xfer_func& val);

	void set_fd(int fd)
	{
		m_v4l2_fd = fd;
	}

	const std::list<v4l2_fmtdesc>& get_fmt_descs() const
	{
		return m_fmt_descs;
	}

	int ioctl_helper(int req);
	int ioctl_helper(int req, void* arg);

 	// V4L2_BUF_TYPE_VIDEO_CAPTURE
 	// V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE
 	// V4L2_BUF_TYPE_VIDEO_OUTPUT
 	// V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE
 	// V4L2_BUF_TYPE_VIDEO_OVERLAY
	bool enum_format_descs(const v4l2_buf_type buf_type);

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

	bool v4l2_probe_ctrl()
	{
		return m_v4l2_ctrl.v4l2_probe_ctrl(m_v4l2_fd);
	}
	bool v4l2_probe_ctrl_ext()
	{
		return m_v4l2_ext_ctrl.v4l2_probe_ctrl(m_v4l2_fd);
	}
	bool get_property_description();

	std::optional<v4l2_buf_type> query_cap();

	std::optional<uint32_t> get_ctrl_id_by_name(const std::string& name) const
	{
		return m_v4l2_ctrl.get_ctrl_id_by_name(name);
	}

	std::optional<v4l2_queryctrl> get_ctrl_by_name(const std::string& name) const
	{
		std::optional<uint32_t> id = m_v4l2_ctrl.get_ctrl_id_by_name(name);
		if( ! id.has_value() )
		{
			return std::optional<v4l2_queryctrl>();
		}

		return m_v4l2_ctrl.get_ctrl_by_id(id.value());
	}

	std::optional<v4l2_queryctrl> get_ctrl_by_id(const uint32_t& id) const
	{
		return m_v4l2_ctrl.get_ctrl_by_id(id);
	}

	std::optional<uint32_t> get_ext_ctrl_id_by_name(const std::string& name) const
	{
		return m_v4l2_ext_ctrl.get_ctrl_id_by_name(name);
	}

	std::optional<v4l2_query_ext_ctrl> get_ext_ctrl_by_name(const std::string& name) const
	{
		std::optional<uint32_t> id = m_v4l2_ext_ctrl.get_ctrl_id_by_name(name);
		if( ! id.has_value() )
		{
			return std::optional<v4l2_query_ext_ctrl>();
		}

		return m_v4l2_ext_ctrl.get_ctrl_by_id(id.value());
	}

	std::optional<v4l2_query_ext_ctrl> get_ext_ctrl_by_id(const uint32_t& id) const
	{
		return m_v4l2_ext_ctrl.get_ctrl_by_id(id);
	}

protected:
	int m_v4l2_fd;

	errno_util m_errno;

	std::list<v4l2_fmtdesc> m_fmt_descs;

	v4l2_ctrl_mgr m_v4l2_ctrl;
	v4l2_ext_ctrl_mgr m_v4l2_ext_ctrl;
};
