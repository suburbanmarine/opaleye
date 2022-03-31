/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "util/v4l2_util.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <cstring>

#include <array>

v4l2_mmap_buffer::v4l2_mmap_buffer()
{
	m_idx = 0;
	m_mmap_ptr = nullptr;
	m_mmap_size = 0;
}
v4l2_mmap_buffer::~v4l2_mmap_buffer()
{
	unmap();
}
bool v4l2_mmap_buffer::init(const int fd, const v4l2_buffer& buf, const v4l2_format& fmt, const size_t idx)
{
	if(m_mmap_ptr != nullptr)
	{
		unmap();
	}

	m_buf = buf;
	m_fmt = fmt;
	m_idx = idx;

	void* mmap_ptr = nullptr;

  switch(fmt.type)
  {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
    {
			mmap_ptr = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
			break;
    }
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
    {
    	//we use our heap allocated planes instead of the user planes, and update the pointer
    	m_planes.resize(m_buf.length);
    	m_buf.m.planes = m_planes.data();

			mmap_ptr = mmap(NULL, buf.m.planes[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.planes[0].m.mem_offset);
			break;
    }
    default:
    {
    	throw std::domain_error("fmt.type not supported");
    	break;
    }
  }

	if (MAP_FAILED == mmap_ptr)
	{
		SPDLOG_ERROR("MMAP failed");
    return false;
	}

	m_mmap_ptr = mmap_ptr;

  switch(fmt.type)
  {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
    {
    	m_mmap_size = (size_t)buf.length;
			break;
    }
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
    {
    	m_mmap_size = (size_t)buf.m.planes[0].length;
			break;
    }
    default:
    {
    	throw std::domain_error("fmt.type not supported");
    	break;
    }
  }

	return true;
}

bool v4l2_mmap_buffer::unmap()
{
	bool ret = true;
	if(m_mmap_ptr)
	{
	  if(-1 == munmap(m_mmap_ptr, m_mmap_size))
		{
			SPDLOG_ERROR("Could not unmap mem");
	    ret = false;
		}
		
		m_mmap_ptr = nullptr;
	}

	return ret;
}

void* v4l2_mmap_buffer::get_data() const
{
	return m_mmap_ptr;
}
size_t v4l2_mmap_buffer::get_size() const
{
return m_mmap_size;
}

uint32_t v4l2_mmap_buffer::get_index() const
{
	return m_idx;
}
uint32_t v4l2_mmap_buffer::get_width() const
{
	uint32_t width = 0;
  switch(m_fmt.type)
  {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
    {
			width = m_fmt.fmt.pix.width;
			break;
    }
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
    {
			width = m_fmt.fmt.pix_mp.width;
			break;
    }
    default:
    {
    	throw std::domain_error("fmt.type not supported");
    	break;
    }
  }

  return width;
}
uint32_t v4l2_mmap_buffer::get_height() const
{
	uint32_t height = 0;
  switch(m_fmt.type)
  {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
    {
			height = m_fmt.fmt.pix.height;
			break;
    }
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
    {
			height = m_fmt.fmt.pix_mp.height;
			break;
    }
    default:
    {
    	throw std::domain_error("fmt.type not supported");
    	break;
    }
  }

  return height;
}
uint32_t v4l2_mmap_buffer::get_bytes_per_line() const
{
	uint32_t bytesperline = 0;
  switch(m_fmt.type)
  {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
    {
			bytesperline = m_fmt.fmt.pix.bytesperline;
			break;
    }
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
    {
			bytesperline = m_fmt.fmt.pix_mp.plane_fmt[0].bytesperline;
			break;
    }
    default:
    {
    	throw std::domain_error("fmt.type not supported");
    	break;
    }
  }

  return bytesperline;
}
uint32_t v4l2_mmap_buffer::get_pixel_format() const
{
	uint32_t pixelformat = 0;
  switch(m_fmt.type)
  {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
    {
			pixelformat = m_fmt.fmt.pix.pixelformat;
			break;
    }
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
    {
			pixelformat = m_fmt.fmt.pix_mp.pixelformat;
			break;
    }
    default:
    {
    	throw std::domain_error("fmt.type not supported");
    	break;
    }
  }

  return pixelformat; 
}

void v4l2_mmap_buffer::reset_buf()
{
	v4l2_buffer temp_buf = m_buf;

	memset(&m_buf, 0, sizeof(m_buf));

	m_buf.index  = temp_buf.index;
	m_buf.type   = temp_buf.type;
	m_buf.memory = temp_buf.memory;
	if(m_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
	{
		m_buf.m.planes = temp_buf.m.planes;
		m_buf.length   = temp_buf.length;
	}
}

v4l2_util::v4l2_util()
{
	m_v4l2_fd = -1;
}
v4l2_util::~v4l2_util()
{

}

int v4l2_util::ioctl_helper(int req)
{
  int ret = 0;

  do
  {
    ret = ioctl(m_v4l2_fd, req);
  } while((ret == -1) && (errno == EINTR));

  return ret;
}

int v4l2_util::ioctl_helper(int req, void* arg)
{
  int ret = 0;

  do
  {
    ret = ioctl(m_v4l2_fd, req, arg);
  } while((ret == -1) && (errno == EINTR));

  return ret;
}

bool v4l2_util::enum_format_descs(const v4l2_buf_type buf_type)
{
	m_fmt_descs.clear();

	for(__u32 i = 0; true; i++)
	{
		v4l2_fmtdesc fmt;
		memset(&fmt, 0, sizeof(fmt));
		fmt.type = buf_type;
		
		fmt.index = i;

		int ret = ioctl_helper(VIDIOC_ENUM_FMT, &fmt);
		if(ret == -1)
		{
			if(errno == EINVAL)
			{
				break;
			}
			else
			{
				SPDLOG_ERROR("ioctl VIDIOC_ENUM_FMT had error: {:d} - {:s}", errno, m_errno.to_str());
				m_fmt_descs.clear();
				return false;
			}
		}

		m_fmt_descs.push_back(fmt);
	}

	return true;
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

	std::array<uint8_t, 1> arr = {0};
	ctrl.p_u8  = arr.data();

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = arr[0];
	return true;
}
bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, uint16_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 2;

	std::array<uint16_t, 1> arr = {0};
	ctrl.p_u16 = arr.data();

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = arr[0];
	return true;
}
bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, uint32_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 4;

	std::array<uint32_t, 1> arr = {0};
	ctrl.p_u32 = arr.data();

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = arr[0];
	return true;
}

bool v4l2_util::v4l2_ctrl_set(v4l2_ext_control* const ctrl)
{
	v4l2_ext_controls ctrls;
	memset(&ctrls, 0, sizeof(ctrls));
	ctrls.which      = V4L2_CTRL_WHICH_CUR_VAL;
	ctrls.ctrl_class = 0; // V4L2_CTRL_ID2CLASS(ctrl->id);
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
	ctrls.ctrl_class = 0; // V4L2_CTRL_ID2CLASS(ctrl->id);
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