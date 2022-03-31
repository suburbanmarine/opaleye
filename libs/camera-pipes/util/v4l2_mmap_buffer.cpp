#include "util/v4l2_mmap_buffer.hpp"

#include <sys/mman.h>

#include <spdlog/spdlog.h>

v4l2_mmap_buffer_base::v4l2_mmap_buffer_base()
{
	memset(&m_buf, 0, sizeof(m_buf));
	memset(&m_fmt, 0, sizeof(m_fmt));
	m_idx = 0;	
}

v4l2_mmap_buffer_base::~v4l2_mmap_buffer_base()
{

}

uint32_t v4l2_mmap_buffer_base::get_index() const
{
	return m_idx;
}
uint32_t v4l2_mmap_buffer_base::get_width() const
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
uint32_t v4l2_mmap_buffer_base::get_height() const
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
uint32_t v4l2_mmap_buffer_base::get_bytes_per_line() const
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
uint32_t v4l2_mmap_buffer_base::get_pixel_format() const
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

v4l2_mmap_buffer::v4l2_mmap_buffer()
{
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
    	// m_planes.resize(m_buf.length);
    	// m_buf.m.planes = m_planes.data();

    // 	for(__u32 i = 0; i < buf.length; i++)
    // 	{
				// mmap_ptr = mmap(NULL, buf.m.planes[i].length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.planes[i].m.mem_offset);
    // 	}
    	throw std::runtime_error("this is not supported");
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
		SPDLOG_ERROR("MMAP failed: {:s}", m_errno.to_str());
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
			SPDLOG_ERROR("munmap failed: {:s}", m_errno.to_str());
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

void v4l2_mmap_buffer::reset_buf()
{
	v4l2_buffer temp_buf = m_buf;

	memset(&m_buf, 0, sizeof(m_buf));

	m_buf.index  = temp_buf.index;
	m_buf.type   = temp_buf.type;
	m_buf.memory = temp_buf.memory;
	// if(m_buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
	// {
	// 	for(size_t i = 0; i < m_planes.size(); i++)
	// 	{
	// 		memset(m_planes.data() + i, 0, sizeof(m_planes[i]));
	// 	}
	//
	// 	m_buf.m.planes = temp_buf.m.planes;
	// 	m_buf.length   = temp_buf.length;
	// }
}

std::shared_ptr<std::vector<uint8_t>> v4l2_mmap_buffer::copy_to_vec() const
{
	std::shared_ptr<std::vector<uint8_t>> vec = std::make_shared<std::vector<uint8_t>>(get_bytes_used());

	uint8_t* start_mmap_ptr = (uint8_t*)get_data();
	std::copy(start_mmap_ptr, start_mmap_ptr + get_bytes_used(), vec->begin());

	return vec;
}