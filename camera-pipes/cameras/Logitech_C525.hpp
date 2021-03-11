#pragma once

#include "cameras/UVC_base.hpp"

#include <boost/format.hpp>

#include <iostream>
#include <fstream>

class Logitech_C525 : public UVC_base
{
public:
	Logitech_C525()
	{

	}

	~Logitech_C525() override
	{

	}

	static void dispatch_frame_cb(uvc_frame* frame_ptr, void* ctx)
	{
		static_cast<Logitech_C525*>(ctx)->frame_cb(frame_ptr);
	}

	void frame_cb(uvc_frame* frame_ptr)
	{
		std::cout << "frame: " << frame_ptr->sequence << std::endl;

		std::string fname = boost::str(boost::format("/home/rounin/image-%d.jpg") % frame_ptr->sequence);
		FILE* file = fopen(fname.c_str(), "w+b");
		if(file)
		{
			fwrite(frame_ptr->data, 1, frame_ptr->data_bytes, file);
		}
		else
		{
			std::cout << "no open" << std::endl;			
		}

		switch(frame_ptr->frame_format)
		{
			case UVC_FRAME_FORMAT_YUYV:
			{
				break;
			}
			case UVC_FRAME_FORMAT_RGB:
			{
				break;
			}
			case UVC_FRAME_FORMAT_MJPEG:
			{
				break;
			}
			default:
			{
				break;
			}
		}
	}

	bool open() override
	{
		return UVC_base::open(0x046d, 0x085e);
	}

	bool start() override
	{
		uvc_error_t ret = uvc_get_stream_ctrl_format_size(
			m_dev_hndl,
			&m_ctrl,
			UVC_FRAME_FORMAT_MJPEG,
			640, 480, 30           /* width, height, fps */
			);
		
		if (ret < 0) {
			uvc_perror(ret, "uvc_get_stream_ctrl_format_size");
			return false;
		}

		/* Print out the result */
		uvc_print_stream_ctrl(&m_ctrl, stderr);

		// UVC_AUTO_EXPOSURE_MODE_APERTURE_PRIORITY
		uvc_set_ae_mode(m_dev_hndl, 2);

		// TODO look at usb spec and figure out what these are
		// TODO AF mode
		uvc_set_focus_auto(m_dev_hndl, 0);

		ret = uvc_start_streaming(m_dev_hndl, &m_ctrl, Logitech_C525::dispatch_frame_cb, this, 0);
		if (ret < 0) {
			uvc_perror(ret, "uvc_get_stream_ctrl_format_size");
			return false;
		}

		return true;
	}

	bool stop() override
	{
		uvc_stop_streaming(m_dev_hndl);
		return true;
	}

	bool close() override
	{
		return UVC_base::close();
	}

protected:
	uvc_stream_ctrl_t m_ctrl;
};
