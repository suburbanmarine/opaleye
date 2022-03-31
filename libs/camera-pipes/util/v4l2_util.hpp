/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "opaleye-util/errno_util.hpp"

#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <cstdint>

#include <list>
#include <vector>

class v4l2_mmap_buffer
{
public:
	v4l2_mmap_buffer();
	~v4l2_mmap_buffer();
	bool init(const int fd, const v4l2_buffer& buf, const v4l2_format& fmt, const size_t idx);
	bool unmap();

    void* get_data() const;  // mmap buffer
    size_t get_size() const; // mmap size

    uint32_t get_index() const;         // mmap buffer index
    uint32_t get_width() const;         // px width
    uint32_t get_height() const;        // px height
    uint32_t get_bytes_per_line() const;
    uint32_t get_pixel_format() const;  // format code

    uint32_t get_bytes_used() const  // number of bytes with actual data within the larger mmap area
    {
    	return m_buf.bytesused;
    }

    //reset buf ecept for idx and type, ready to be enqueued
    void reset_buf();

	const v4l2_buffer& get_buf() const
	{
		return m_buf;
	}
	const v4l2_format& get_fmt() const
	{
		return m_fmt;
	}

	void set_buf(const v4l2_buffer& buf)
	{
		m_buf = buf;
	}

protected:
	v4l2_buffer m_buf;
	std::vector<v4l2_plane> m_planes;
	v4l2_format m_fmt;
	size_t      m_idx;

	void*  m_mmap_ptr;
	size_t m_mmap_size;

};

class v4l2_util
{
public:

	v4l2_util();
	~v4l2_util();

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
protected:
	int m_v4l2_fd;

	errno_util m_errno;

	std::list<v4l2_fmtdesc> m_fmt_descs;
};