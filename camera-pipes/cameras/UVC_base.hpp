#pragma once

#include "libuvc/libuvc.h"

#include <memory>
#include <functional>

class UVC_base
{
public:
	UVC_base();
	virtual ~UVC_base();

	virtual bool open();
	virtual bool open(const uint16_t vendor_id, const uint16_t product_id);

	virtual bool start() = 0;
	virtual bool stop()  = 0;

	virtual bool close();

	static std::shared_ptr<uvc_frame_t> allocate_frame(const size_t data_bytes);

	void register_callback(const std::function<void(const std::shared_ptr<uvc_frame_t>& frame)>& cb);

protected:

	uvc_context_t*       m_uvc;
	uvc_device_t*        m_dev;
	uvc_device_handle_t* m_dev_hndl;

	std::function<void(const std::shared_ptr<uvc_frame_t>& frame)> m_frame_cb;
};
