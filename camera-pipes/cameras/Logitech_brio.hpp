#pragma once

#include "cameras/UVC_base.hpp"

#include <iostream>
#include <fstream>
#include <mutex>
#include <memory>

class Logitech_brio : public UVC_base
{
public:
	Logitech_brio();

	~Logitech_brio() override;

	static void dispatch_frame_cb(uvc_frame_t* frame_ptr, void* ctx);

	void frame_cb(uvc_frame_t* new_frame_ptr);

	bool open() override;

	bool start() override;

	bool stop() override;

	bool close() override;

protected:
	uvc_stream_ctrl_t m_ctrl;
};
