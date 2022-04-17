/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "http-bridge/http_req_callback_base.hpp"
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

	//top level handler
	void handle(FCGX_Request* const request) override;

	void handle_index(FCGX_Request* const request);

	void handle_temperature(FCGX_Request* const request);
	void handle_pressure(FCGX_Request* const request);
protected:

	std::shared_ptr<sensor_thread> m_sensors;
};