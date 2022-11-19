#pragma once

#include "elements/gst_clock_base.hpp"

#include <array>
#include <string>

class sys_clock : public gst_clock_base
{
	public:

	sys_clock();
	~sys_clock() override;

	bool init() override;

	protected:
};

