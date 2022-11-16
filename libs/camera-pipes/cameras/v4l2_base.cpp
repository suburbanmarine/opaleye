#include "v4l2_base.hpp"

#include "opaleye-util/chrono_util.hpp"

#include "util/v4l2_metadata.hpp"

#include <boost/property_tree/ptree.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

v4l2_base::v4l2_base()
{
	m_fd = -1;
	memset(&m_cap, 0, sizeof(m_cap));
}
v4l2_base::~v4l2_base()
{
	close();
}

bool v4l2_base::open(const char dev_path[])
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
    else
    {
      SPDLOG_ERROR("ioctl VIDIOC_QUERYCAP failed: {:s}", m_errno.to_str());
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

  if( ! m_v4l2_util.v4l2_probe_ctrl_ext() )
  {
    SPDLOG_ERROR("Could not probe ctrls");
    return false; 
  }

  return m_fd != -1;
}
bool v4l2_base::close()
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

bool v4l2_base::start_streaming()
{
  for(auto b : m_buf_by_idx)
  {
    b.second->reset_buf();
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
bool v4l2_base::stop_streaming()
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

bool v4l2_base::wait_for_frame(const std::chrono::microseconds& timeout)
{
  return wait_for_frame(timeout, FrameCallback());
}

bool v4l2_base::wait_for_frame(const std::chrono::microseconds& timeout, const FrameCallback& cb)
{
  fd_set fdset;
  FD_ZERO(&fdset);
  FD_SET(m_fd, &fdset);

  timeval tv = chrono_to_timeval(timeout);

  int ret = select(m_fd + 1, &fdset, NULL, NULL, &tv);
  if(ret == -1)
  {
    SPDLOG_ERROR("select failed: {:s}", m_errno.to_str());
  }

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
      
      //update buf metadata
      new_frame->set_buf(buf);
      
      SPDLOG_TRACE("Got new frame idx {:d} ptr {}", new_frame->get_index(), fmt::ptr(new_frame->get_data()));
      if(cb)
      {
        cb(new_frame);
      }

      //return buffer
      {
        new_frame->reset_buf();
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

bool v4l2_base::frame_meta_to_ptree(const ConstMmapFramePtr& frame, boost::property_tree::ptree* const out_meta)
{
  if( ! (frame && out_meta) )
  {
    return false;
  }

  boost::property_tree::ptree buf_tree;
  v4l2_metadata::v4l2_buffer_to_json(frame->get_buf(), &buf_tree);

  boost::property_tree::ptree fmt_tree;
  v4l2_metadata::v4l2_format_to_json(frame->get_fmt(), &fmt_tree);

  out_meta->put_child("buf", buf_tree);
  out_meta->put_child("fmt", fmt_tree);

  return true;
}
