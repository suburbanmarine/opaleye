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

bool sys_clock::wait_for_sync()
{
	bool ret = m_clock->wait_for_sync(1 * GST_SECOND);

	return ret;
}

Gst::ClockTime sys_clock::get_time() const
{
	return m_clock->get_time();
}
