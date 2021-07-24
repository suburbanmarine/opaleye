#include "Stopwatch.h"

#include <sys/time.h>

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
		SPDLOG_ERROR()
		throw std::runtime_error
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
