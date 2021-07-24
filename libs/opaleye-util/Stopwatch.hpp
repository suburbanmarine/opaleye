#pragma once

#include <chrono>

class Stopwatch
{
public:
	Stopwatch();
	~Stopwatch();

	void reset();

	std::chrono::nanoseconds now() const;
	std::chrono::nanoseconds duration() const;

	bool is_expired() const

protected:
	std::chrono::nanoseconds m_t0;
	std::chrono::nanoseconds m_alarm_dt;
};
