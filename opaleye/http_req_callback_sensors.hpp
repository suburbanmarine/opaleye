#pragma once

#include "http_req_callback_base.hpp"
#include "sensor_thread.hpp"

#define NO_FCGI_DEFINES 1
#include <fcgi_config.h>
#include <fcgiapp.h>

class http_req_callback_sensors : public http_req_callback_base
{
public:

	http_req_callback_sensors()
	{

	}

	~http_req_callback_sensors() override
	{

	}

	void init(const std::shared_ptr<sensor_thread>& sensors)
	{
		m_sensors = sensors;
	}

	void handle(FCGX_Request* const request) override;
protected:

	std::shared_ptr<sensor_thread> m_sensors;
};