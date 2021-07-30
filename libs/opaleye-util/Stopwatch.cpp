#include "Stopwatch.hpp"

#include <spdlog/spdlog.h>

#include <sys/time.h>

#include <stdexcept>

Stopwatch::Stopwatch()
{
	m_clk_id = CLOCK_MONOTONIC;

	m_t0       = std::chrono::nanoseconds::zero();
	m_alarm_dt = std::chrono::nanoseconds::zero();
}
Stopwatch::Stopwatch(clockid_t clk_id)
{
	m_clk_id = clk_id;

	m_t0       = std::chrono::nanoseconds::zero();
	m_alarm_dt = std::chrono::nanoseconds::zero();
}
Stopwatch::~Stopwatch()
{

}

void Stopwatch::reset()
{
	m_t0 = now();
}

std::chrono::nanoseconds Stopwatch::now() const
{
	timespec ts;
	int ret = clock_gettime(m_clk_id, &ts);

	if(ret != 0)
	{
		SPDLOG_ERROR("clock_gettime failed");
		throw std::runtime_error("clock_gettime failed");
	}

	return std::chrono::seconds(ts.tv_sec) + std::chrono::nanoseconds(ts.tv_nsec);
}

std::chrono::nanoseconds Stopwatch::duration() const
{
	return now() - m_t0;
}

bool Stopwatch::is_expired() const
{
	return duration() >= m_alarm_dt;
}

std::chrono::nanoseconds Stopwatch::time_left() const
{
	const std::chrono::nanoseconds dt = (m_t0 + m_alarm_dt) - now();
	return (dt > std::chrono::nanoseconds::zero()) ? (dt) : (std::chrono::nanoseconds::zero());
}
