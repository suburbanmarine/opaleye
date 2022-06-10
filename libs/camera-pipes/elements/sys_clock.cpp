#include"sys_clock.hpp"

#include <spdlog/spdlog.h>

#include <memory>

sys_clock::sys_clock()
{
	
}

sys_clock::~sys_clock()
{

}

bool sys_clock::init()
{
	m_clock = Gst::SystemClock::obtain();
	m_clock->set_property("clock_type", GST_CLOCK_TYPE_REALTIME);

	return true;
}
