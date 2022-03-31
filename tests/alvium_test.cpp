#include "cameras/Alvium_v4l2.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/bundled/printf.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <thread>
#include <sstream>
#include <string>

static int filenum;

void new_frame_cb(const Alvium_v4l2::ConstMmapFramePtr& frame)
{
	if(frame)
	{
		std::stringstream ss;
		ss << "\tsizeimage: "    << frame->get_fmt().fmt.pix.sizeimage << "\n";
		ss << "\twidth: "        << frame->get_fmt().fmt.pix.width << "\n";
		ss << "\theight: "       << frame->get_fmt().fmt.pix.height << "\n";
		ss << "\tbytesperline: " << frame->get_fmt().fmt.pix.bytesperline << "\n";
		ss << "\tpixelformat: "  << v4l2_util::fourcc_to_str(frame->get_fmt().fmt.pix.pixelformat) << "\n";

		switch(frame->get_fmt().type)
		{
			case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			{
				const v4l2_buffer& frame_buf = frame->get_buf();
				size_t blob_size = frame_buf.bytesused;

				if(frame_buf.field != V4L2_FIELD_NONE)
				{
					SPDLOG_ERROR("unhandled frame field {:d}", frame_buf.field);
				}

				if(frame_buf.flags & V4L2_BUF_FLAG_ERROR)
				{
					SPDLOG_ERROR("frame error flag set");
				}

				if(frame_buf.flags & V4L2_BUF_FLAG_TIMECODE)
				{
					ss << "\ttimecode: " << frame_buf.timecode << "\n";
				}

				switch(frame_buf.flags & V4L2_BUF_FLAG_TIMESTAMP_MASK)
				{
					case V4L2_BUF_FLAG_TIMESTAMP_UNKNOWN:
					{
						ss << fmt::sprintf("\ttimestamp_unknown: %d.%06d\n", frame_buf.timestamp.tv_sec, frame_buf.timestamp.tv_usec);
						break;
					}
					case V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC:
					{
						ss << fmt::sprintf("\ttimestamp_monotonic: %d.%06d\n", frame_buf.timestamp.tv_sec, frame_buf.timestamp.tv_usec);
						break;
					}
					case V4L2_BUF_FLAG_TIMESTAMP_COPY:
					{
						ss << fmt::sprintf("\ttimestamp_copy: %d.%06d\n", frame_buf.timestamp.tv_sec, frame_buf.timestamp.tv_usec);
						break;
					}
					default:
					{
						SPDLOG_ERROR("unhandled TIMESTAMP {:d}", frame_buf.flags & V4L2_BUF_FLAG_TIMESTAMP_MASK);
						break;
					}
				}

				switch(frame_buf.flags & V4L2_BUF_FLAG_TSTAMP_SRC_MASK)
				{
					case V4L2_BUF_FLAG_TSTAMP_SRC_EOF:
					{
						ss << "\ttimestamp_src: eof\n";
						break;
					}
					case V4L2_BUF_FLAG_TSTAMP_SRC_SOE:
					{
						ss << "\ttimestamp_src: soe\n";
						break;
					}
					default:
					{
						SPDLOG_ERROR("unhandled TSTAMP_SRC {:d}", frame_buf.flags & V4L2_BUF_FLAG_TSTAMP_SRC_MASK);
						break;
					}
				}

				SPDLOG_INFO("New Frame Info:\n{:s}", ss.str());

				std::string fname = fmt::format("/tmp/temp_{:d}.bin", filenum);
				filenum++;
				
				{
					int fd = open(fname.c_str(), O_WRONLY | O_CREAT, 0644);
					if(fd < 0)
					{
						SPDLOG_ERROR("failed to open file");
					}

					int ret = write(fd, frame->get_data(), frame->get_bytes_used());
					if(ret != frame->get_bytes_used())
					{
						SPDLOG_ERROR("write failed: {:d}", ret);
					}

					ret = close(fd);
					if(ret != 0)
					{
						SPDLOG_ERROR("close failed");
					}
				}

				break;
			}
			default:
			{
				SPDLOG_ERROR("unhandled frame type");
				break;
			}
		}
	}
}

int main()
{
	Alvium_v4l2 cam;

	if( ! cam.open("/dev/video0") )
	{
		SPDLOG_ERROR("cam.open failed");
		return -1;
	}

	if( ! cam.init("cam0") )
	{
		SPDLOG_ERROR("cam.init failed");
		return -1;
	}

	if( ! cam.start_streaming() )
	{
		SPDLOG_ERROR("cam.start_streaming() failed");
		return -1;
	}

	if( ! cam.set_sw_trigger() )
	{
		SPDLOG_ERROR("cam.set_sw_trigger() failed");
		return -1;
	}

	
	for(int i = 0; i < 10; i++)
	{
		if( ! cam.send_software_trigger() )
		{
			SPDLOG_ERROR("cam.send_software_trigger() failed");
			return -1;
		}

		if( ! cam.wait_for_frame(std::chrono::milliseconds(250), new_frame_cb) )
		{
			SPDLOG_ERROR("cam.wait_for_frame() failed");
			return -1;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	if( ! cam.stop_streaming() )
	{
		SPDLOG_ERROR("cam.stop_streaming() failed");
		return -1;
	}

	if( ! cam.close() )
	{
		SPDLOG_ERROR("cam.close() failed");
		return -1;
	}

	return 0;
}