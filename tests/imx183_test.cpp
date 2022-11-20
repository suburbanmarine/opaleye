#include "cameras/imx183_v4l2.hpp"
#include "util/v4l2_metadata.hpp"

#include "gpio_thread.hpp"

#include "opaleye-util/Ptree_util.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

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
#include <iostream>

static int filenum;
static bool write_to_disk;
static int num_trigger;

void new_frame_cb(const v4l2_base::ConstMmapFramePtr& frame)
{
	timespec cb_time;
	clock_gettime(CLOCK_MONOTONIC, &cb_time);

	if(frame)
	{
		num_trigger++;

		std::stringstream ss;
		ss << "\tsequence:  "    << frame->get_buf().sequence             << "\n";
		ss << "\tsizeimage: "    << frame->get_fmt().fmt.pix.sizeimage    << "\n";
		ss << "\twidth: "        << frame->get_fmt().fmt.pix.width        << "\n";
		ss << "\theight: "       << frame->get_fmt().fmt.pix.height       << "\n";
		ss << "\tbytesperline: " << frame->get_fmt().fmt.pix.bytesperline << "\n";
		ss << "\tpixelformat: "  << v4l2_util::fourcc_to_str(frame->get_fmt().fmt.pix.pixelformat) << "\n";

		{
			boost::property_tree::ptree meta_tree;

			boost::property_tree::ptree buf_tree;
			v4l2_metadata::v4l2_buffer_to_json(frame->get_buf(), &buf_tree);

			boost::property_tree::ptree fmt_tree;
			v4l2_metadata::v4l2_format_to_json(frame->get_fmt(), &fmt_tree);

			meta_tree.put_child("buf", buf_tree);
			meta_tree.put_child("fmt", fmt_tree);

			SPDLOG_INFO("Metadata:\n{:s}", 
				Ptree_util::ptree_to_json_str(meta_tree)
			);
		}

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
						ss << fmt::sprintf("\tcallback timestamp_monotonic: %d.%09d\n", cb_time.tv_sec, cb_time.tv_nsec);
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

				if(write_to_disk)
				{
					std::string fname = fmt::format("/tmp/temp_{:d}.{:s}.bin", 
						filenum,
						v4l2_util::fourcc_to_str(frame->get_fmt().fmt.pix.pixelformat)
						);
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

int main(int argc, char* argv[])
{
	//Parse program options
	boost::program_options::variables_map vm;
	{
		//register options
		namespace bpo = boost::program_options;
		bpo::options_description desc("Options"); 
	    desc.add_options() 
			("help"      , "Print usage information and exit")
			("device"    , bpo::value<std::string>()->default_value("/dev/video0"), "device to open")
			("fourcc"    , bpo::value<std::string>()->default_value("RG12"),        "fcc code to ask for image format, try RG12")
			("disk"      , bpo::bool_switch(),                                      "Write to disk")
			("verbose,v" , bpo::bool_switch(),                                      "Verbose")
			("num_frames", bpo::value<int>()->default_value(10),                    "Number of frames to grab")
			;

		//Parse options
	    try
	    {
	    	bpo::store(bpo::parse_command_line(argc, argv, desc), vm);

			if(vm.count("help"))
			{
				std::cout << desc << std::endl;
				return 0;
			}

	    	bpo::notify(vm);
	    }
	    catch(const bpo::error& e)
	    {
	    	std::cout << e.what() << std::endl;
		  	std::cout << desc << std::endl;
			return -1;
	    }
	}

	if(vm["verbose"].as<bool>())
	{
		spdlog::set_level(spdlog::level::trace);
	}
	else
	{
		spdlog::set_level(spdlog::level::info);
	}

	write_to_disk = vm["disk"].as<bool>();

	std::string fourcc = vm["fourcc"].as<std::string>();
	if(fourcc.size() != 4)
	{
	  	std::cout << "FCC code must be 4 chars long" << std::endl;
		return -1;
	}

	imx183_v4l2 cam;

	if( ! cam.open(vm["device"].as<std::string>().c_str()) )
	{
		SPDLOG_ERROR("cam.open({:s}) failed", vm["device"].as<std::string>());
		return -1;
	}

	if( ! cam.init("cam0", v4l2_fourcc(fourcc[0], fourcc[1], fourcc[2], fourcc[3])) )
	{
		SPDLOG_ERROR("cam.init failed");
		return -1;
	}

	if( ! cam.get_v4l2_util().has_any_ctrl() )
	{
		SPDLOG_WARN("cam has no controls");
	}

	v4l2_util::JsonDocPtr prop_desc = cam.get_v4l2_util().get_property_description();
	if(prop_desc)
	{
		rapidjson::StringBuffer buf;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
		prop_desc->Accept(writer);

		SPDLOG_INFO("properties: {:s}", buf.GetString());
	}
	else
	{
		SPDLOG_WARN("geting props description failed");
	}

	if( ! cam.start_streaming() )
	{
		SPDLOG_ERROR("cam.start_streaming() failed");
		return -1;
	}

	int num_frames = vm["num_frames"].as<int>();
	bool keep_going = num_frames != 0;
	while(keep_going)
	{
		if( ! cam.wait_for_frame(std::chrono::milliseconds(250), new_frame_cb) )
		{
			SPDLOG_ERROR("cam.wait_for_frame() failed");
			return -1;
		}

		if(num_frames < 0)
		{
			keep_going = true;
		}
		else if(num_trigger >= num_frames)
		{
			keep_going = false;
			break;
		}
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