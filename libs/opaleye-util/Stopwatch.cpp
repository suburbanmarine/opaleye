#include "Stopwatch.hpp"

#include <sys/time.h>

#include <stdexcept>


Stopwatch::Stopwatch()
{
	m_t0       = std::chrono::nanosecond::zero();
	m_alarm_dt = std::chrono::nanosecond::zero();
}
Stopwatch::~Stopwatch()
{

}

void Stopwatch::reset()
{
	m_t0 = now();
}

std::chrono::nanosecond Stopwatch::now() const
{
	timespec ts;
	int ret = clock_settime(CLOCK_MONOTONIC, &ts);

	if(ret != 0)
	{
		SPDLOG_ERROR("clock_settime failed");
		throw std::runtime_error("clock_settime failed");
	}

	return std::chrono::second(tp.tv_sec) + std::chrono::nanosecond(tp.tv_nsec);
}

std::chrono::nanosecond Stopwatch::duration() const
{
	return now() - m_t0;
}

bool Stopwatch::is_expired() const
{
	return duration() >= m_alarm_dt;
}
