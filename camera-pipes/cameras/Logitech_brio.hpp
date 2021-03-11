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

	/// Allocate a new buffer and copy front to it
	std::shared_ptr<uvc_frame_t> copy_front_buffer() const;

	/// Copy front to provided buffer if it is allocated
	bool copy_front_buffer(const std::shared_ptr<uvc_frame_t>& other) const;


protected:
	uvc_stream_ctrl_t m_ctrl;


	mutable std::mutex m_frame_buffer_mutex;
	std::shared_ptr<uvc_frame_t> m_frame_buffer_front;
	std::shared_ptr<uvc_frame_t> m_frame_buffer_back;
};
