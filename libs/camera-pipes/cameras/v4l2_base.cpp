#include "v4l2_base.hpp"

#include "opaleye-util/chrono_util.hpp"

#include "util/v4l2_metadata.hpp"

#include <boost/property_tree/ptree.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

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
