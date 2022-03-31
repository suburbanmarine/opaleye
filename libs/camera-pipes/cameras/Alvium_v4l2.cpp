#include "Alvium_v4l2.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

///
/// Private alvium ioctls
///

#define VIDIOC_TRIGGER_MODE_OFF             _IO('V',  BASE_VIDIOC_PRIVATE + 20)
#define VIDIOC_TRIGGER_MODE_ON              _IO('V',  BASE_VIDIOC_PRIVATE + 21)
#define VIDIOC_S_TRIGGER_ACTIVATION         _IOW('V', BASE_VIDIOC_PRIVATE + 22, int)
#define VIDIOC_G_TRIGGER_ACTIVATION         _IOR('V', BASE_VIDIOC_PRIVATE + 23, int)
#define VIDIOC_S_TRIGGER_SOURCE             _IOW('V', BASE_VIDIOC_PRIVATE + 24, int)
#define VIDIOC_G_TRIGGER_SOURCE             _IOR('V', BASE_VIDIOC_PRIVATE + 25, int)
#define VIDIOC_TRIGGER_SOFTWARE             _IO('V',  BASE_VIDIOC_PRIVATE + 26)

namespace Alvium_CSI
{
  enum class v4l2_trigger_source
  {
    V4L2_TRIGGER_SOURCE_SOFTWARE = 0,
    V4L2_TRIGGER_SOURCE_LINE0    = 1,
    V4L2_TRIGGER_SOURCE_LINE1    = 2,
    V4L2_TRIGGER_SOURCE_LINE2    = 3,
    V4L2_TRIGGER_SOURCE_LINE3    = 4
  };
  enum class v4l2_trigger_activation
  {
    V4L2_TRIGGER_ACTIVATION_RISING_EDGE  = 0,
    V4L2_TRIGGER_ACTIVATION_FALLING_EDGE = 1,
    V4L2_TRIGGER_ACTIVATION_ANY_EDGE     = 2,
    V4L2_TRIGGER_ACTIVATION_LEVEL_HIGH   = 3,
    V4L2_TRIGGER_ACTIVATION_LEVEL_LOW    = 4
  };
}

Alvium_v4l2::Alvium_v4l2()
{
  m_fd = -1;
  memset(&m_cap, 0, sizeof(m_cap));
}

Alvium_v4l2::~Alvium_v4l2()
{
  close();
}

bool Alvium_v4l2::open(const char dev_path[])
{
  if(m_fd != -1)
  {
    SPDLOG_ERROR("fd already set");
    return false;
  }

  m_fd = ::open(dev_path, O_RDWR | O_NONBLOCK, 0);
  if(m_fd == -1)
  {
    SPDLOG_ERROR("open had error: {:d} - {:s}", errno, m_errno.to_str());
  }

  m_v4l2_util.set_fd(m_fd);

  int ret = m_v4l2_util.ioctl_helper(VIDIOC_QUERYCAP, &m_cap);
  if(ret == -1)
  {
    if(errno == EINVAL)
    {
      SPDLOG_ERROR("Device {:s} is not a V4L2 device");
      close();
      return false;
    }
  }

  if( ! (m_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) || (m_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) )
  {
      SPDLOG_ERROR("Device {:s} is not a video capture device");
      close();
      return false;
  }

  if( ! (m_cap.capabilities & V4L2_CAP_STREAMING)  )
  {
      SPDLOG_ERROR("Device {:s} does not support streaming i/o");
      close();
      return false;
  }

  if(m_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
  {
    m_v4l2_util.enum_format_descs(V4L2_BUF_TYPE_VIDEO_CAPTURE);
    m_buffer_type = (v4l2_buf_type)V4L2_CAP_VIDEO_CAPTURE;
  }
  else if(m_cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
  {
    m_v4l2_util.enum_format_descs(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    m_buffer_type = (v4l2_buf_type)V4L2_CAP_VIDEO_CAPTURE_MPLANE;
  }
  else
  {
    SPDLOG_ERROR("Discovered unexpected VIDIOC_QUERYCAP capabilities: {:d}", m_cap.capabilities);
    return false;
  }

  return m_fd != -1;
}
bool Alvium_v4l2::close()
{
  if(m_fd == -1)
  {
    return true;
  }

  stop_streaming();

  m_buf_by_idx.clear();
  m_buf_by_ptr.clear();

  {
    v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count  = 0;
    req.type   = m_buffer_type;
    req.memory = V4L2_MEMORY_MMAP;
    if (-1 == m_v4l2_util.ioctl_helper(VIDIOC_REQBUFS, &req))
    {
      SPDLOG_ERROR("Could not get buffers from device, {:s}", m_errno.to_str());
      return false;
    }
  }

  int ret = ::close(m_fd);
  m_fd = -1;

  if(ret == -1)
  {
    SPDLOG_ERROR("close had error: {:d} - {:s}", errno, m_errno.to_str());
  }

  return ret == 0;
}
bool Alvium_v4l2::init(const char name[])
{

  if(m_v4l2_util.get_fmt_descs().empty())
  {
    SPDLOG_ERROR("Format descriptions not set");
    return false;
  }

  v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = m_buffer_type;
  if (-1 == m_v4l2_util.ioctl_helper(VIDIOC_G_FMT, &fmt))
  {
      SPDLOG_ERROR("ioctl VIDIOC_G_FMT failed");
      return false;
  }

  // Bayer format (8/10/12 bit)
  // Pixel Format: 'RGGB' - V4L2_PIX_FMT_SRGGB8
  // Name        : 8-bit Bayer RGRG/GBGB
  // Pixel Format: 'JXR0' - v4l2_fourcc('J', 'X', 'R', '0')
  // Name        : 10-bit/16-bit Bayer RGRG/GBGB
  // Pixel Format: 'JXR2' - v4l2_fourcc('J', 'X', 'R', '2')
  // Name        : 12-bit/16-bit Bayer RGRG/GBGB
  
  // Luma Format (8bit)
  // Pixel Format: 'VYUY' - V4L2_PIX_FMT_VYUY
  // Name        : VYUY 4:2:2

  // Color Format (8bit)
  // Pixel Format: 'XR24' - V4L2_PIX_FMT_XBGR32
  // Name        : 32-bit BGRX 8-8-8-8
  __u32 pixel_format = V4L2_PIX_FMT_XBGR32;

  switch(fmt.type)
  {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
    {
      fmt.fmt.pix.pixelformat = pixel_format;
      break;
    }
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
    {
      fmt.fmt.pix_mp.pixelformat = pixel_format;
      break;
    }
    default:
    {
      SPDLOG_ERROR("fmt.type not supported");
      return false;
      break;
    }
  }

  if (-1 == m_v4l2_util.ioctl_helper(VIDIOC_S_FMT, &fmt))
  {
    SPDLOG_ERROR("ioctl VIDIOC_S_FMT failed");
    return false;
  }
 
  fmt.type = m_buffer_type;
  if (-1 == m_v4l2_util.ioctl_helper(VIDIOC_G_FMT, &fmt))
  {
      SPDLOG_ERROR("ioctl VIDIOC_G_FMT failed");
      return false;
  }


  __u32 sizeimage    = 0;
  __u32 width        = 0;
  __u32 height       = 0;
  __u32 bytesperline = 0;
  __u32 pixelformat  = 0;
  __u8 num_planes    = 0;

  switch(fmt.type)
  {
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
    {
      sizeimage    = fmt.fmt.pix.sizeimage;
      width        = fmt.fmt.pix.width;
      height       = fmt.fmt.pix.height;
      bytesperline = fmt.fmt.pix.bytesperline;
      pixelformat  = fmt.fmt.pix.pixelformat;
      break;
    }
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
    {
      num_planes   = fmt.fmt.pix_mp.num_planes;
      width        = fmt.fmt.pix_mp.width;
      height       = fmt.fmt.pix_mp.height;
      pixelformat  = fmt.fmt.pix_mp.pixelformat;
      bytesperline = fmt.fmt.pix_mp.plane_fmt[0].bytesperline;
      sizeimage    = fmt.fmt.pix_mp.plane_fmt[0].sizeimage;
      break;
    }
    default:
    {
      SPDLOG_ERROR("fmt.type not supported");
      return false;
      break;
    }
  }

  v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req));
  req.count  = 2;
  req.type   = m_buffer_type;
  req.memory = V4L2_MEMORY_MMAP;
  if (-1 == m_v4l2_util.ioctl_helper(VIDIOC_REQBUFS, &req))
  {
    SPDLOG_ERROR("Could not get buffers from device, {:s}", m_errno.to_str());
    return false;
  }

  if (req.count <= 0)
  {
    SPDLOG_ERROR("Could not get buffers from device");
    return false;
  }

  if (req.count < 2)
  {
    SPDLOG_WARN("Could only get {:d} buffers from device", req.count);
  }

  for(__u32 i = 0; i < req.count; i++)
  {

    v4l2_buffer buf;
    v4l2_plane planes[num_planes];
    memset(&buf,    0, sizeof(buf));
    memset(&planes, 0, sizeof(planes));

    switch(fmt.type)
    {
      case V4L2_BUF_TYPE_VIDEO_CAPTURE:
      {
        buf.m.planes = nullptr;
        buf.length   = num_planes;

        break;
      }
      case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
      {
        buf.m.planes = planes;
        buf.length   = num_planes;
        break;
      }
      default:
      {
        throw std::domain_error("fmt.type not supported");
        break;
      }
    }

    buf.type    = m_buffer_type;
    buf.memory  = V4L2_MEMORY_MMAP;
    buf.index   = i;

    if(-1 == m_v4l2_util.ioctl_helper(VIDIOC_QUERYBUF, &buf))
    {
      SPDLOG_ERROR("ioctl VIDIOC_QUERYBUF failed: {:s}", m_errno.to_str());
      return false;
    }


    std::shared_ptr<v4l2_mmap_buffer> mmap_buf = std::make_shared<v4l2_mmap_buffer>();
    if( ! mmap_buf->init(m_fd, buf, fmt, i) )
    {
      SPDLOG_ERROR("Could not init buffer");
      return false;
    }

    m_buf_by_idx.insert(std::make_pair(mmap_buf->get_index(), mmap_buf));
    m_buf_by_ptr.insert(std::make_pair(mmap_buf->get_data(),  mmap_buf));
  }

  return true;
}

bool Alvium_v4l2::start_streaming()
{
  for(auto b : m_buf_by_idx)
  {
    v4l2_buffer buf = b.second->get_buf();
    if (-1 == m_v4l2_util.ioctl_helper(VIDIOC_QBUF, &buf))
    {
      SPDLOG_ERROR("ioctl VIDIOC_QBUF failed: {:s}", m_errno.to_str());
      return false;
    }
  }

  v4l2_buf_type type = m_buffer_type;
  if(-1 == m_v4l2_util.ioctl_helper(VIDIOC_STREAMON, &type))
  {
    SPDLOG_ERROR("ioctl VIDIOC_STREAMON failed: {:s}", m_errno.to_str());
    return false;
  }

  return true;
}
bool Alvium_v4l2::stop_streaming()
{
  //TODO sync callbacks

  v4l2_buf_type type = m_buffer_type;
  if(-1 == m_v4l2_util.ioctl_helper(VIDIOC_STREAMOFF, &type))
  {
    SPDLOG_ERROR("ioctl VIDIOC_STREAMON failed: {:s}", m_errno.to_str());
    return false;
  }

  return true;
}

bool Alvium_v4l2::wait_for_frame()
{
  fd_set fdset;
  FD_ZERO(&fdset);
  FD_SET(m_fd, &fdset);

  timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = 0;

  int nResult = select(m_fd + 1, &fdset, NULL, NULL, &tv);

  if (FD_ISSET(m_fd, &fdset) != 0)
  {
    //get buffer
    v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type   = m_buffer_type;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == m_v4l2_util.ioctl_helper(VIDIOC_DQBUF, &buf))
    {
      SPDLOG_ERROR("ioctl VIDIOC_DQBUF failed: {:s}", m_errno.to_str());
      return false;
    }

    auto it = m_buf_by_idx.find(buf.index);
    if(it == m_buf_by_idx.end())
    {
      SPDLOG_ERROR("got invalid buffer index");
      return false;
    }

    {
      //process buffer
      std::shared_ptr<v4l2_mmap_buffer> new_frame = it->second;
      
      SPDLOG_INFO("Got new frame idx {:d} ptr {}", new_frame->get_index(), fmt::ptr(new_frame->get_data()));

      //return buffer
      {
        v4l2_buffer buf = new_frame->get_buf();
        if (-1 == m_v4l2_util.ioctl_helper(VIDIOC_QBUF, &buf))
        {
          SPDLOG_ERROR("ioctl VIDIOC_QBUF failed: {:s}", m_errno.to_str());
          return false;
        }
      }
    }
  }

  return true;
}

bool Alvium_v4l2::set_free_trigger()
{
  // Set trigger mode
  if( m_v4l2_util.ioctl_helper(VIDIOC_TRIGGER_MODE_OFF ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_TRIGGER_MODE_OFF failed: {:s}", m_errno.to_str());
      return false;
  }

  return true;
}
bool Alvium_v4l2::set_sw_trigger()
{
  // Set trigger mode
  if( m_v4l2_util.ioctl_helper(VIDIOC_TRIGGER_MODE_ON ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_TRIGGER_MODE_ON failed: {:s}", m_errno.to_str());
      return false;
  }

  // Set trigger source to software
  int source = (int)Alvium_CSI::v4l2_trigger_source::V4L2_TRIGGER_SOURCE_SOFTWARE;
  if( m_v4l2_util.ioctl_helper(VIDIOC_S_TRIGGER_SOURCE, &source ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_S_TRIGGER_SOURCE / V4L2_TRIGGER_SOURCE_SOFTWARE failed: {:s}", m_errno.to_str());
      return false;
  }

  return true;
}
bool Alvium_v4l2::set_hw_trigger()
{
  // Set trigger mode
  if( m_v4l2_util.ioctl_helper(VIDIOC_TRIGGER_MODE_ON ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_TRIGGER_MODE_ON failed: {:s}", m_errno.to_str());
      return false;
  }

  // Set trigger source
  int source = (int)Alvium_CSI::v4l2_trigger_source::V4L2_TRIGGER_SOURCE_LINE0;
  if( m_v4l2_util.ioctl_helper(VIDIOC_S_TRIGGER_SOURCE, &source ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_S_TRIGGER_SOURCE / V4L2_TRIGGER_SOURCE_LINE0 failed: {:s}", m_errno.to_str());
      return false;
  }

  // Set trigger activation
  int activation = (int)Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_RISING_EDGE;
  // int activation = (int)Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_FALLING_EDGE;
  // int activation = (int)Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_ANY_EDGE;
  // int activation = (int)Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_LEVEL_HIGH;
  // int activation = (int)Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_LEVEL_LOW;
  if( m_v4l2_util.ioctl_helper(VIDIOC_S_TRIGGER_ACTIVATION, &activation ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_S_TRIGGER_ACTIVATION failed: {:s}", m_errno.to_str());
      return false;
  }

  return true;
}

bool Alvium_v4l2::send_software_trigger()
{
  // Set trigger mode
  if( m_v4l2_util.ioctl_helper(VIDIOC_TRIGGER_SOFTWARE ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_TRIGGER_SOFTWARE failed: {:s}", m_errno.to_str());
      return false;
  }

  return true;
}
