#pragma once

#include <chrono>

class Stopwatch
{
public:
	Stopwatch();
	~Stopwatch();

	void reset()

	std::chrono::nanosecond now() const;
	std::chrono::nanosecond duration() const;

	bool is_expired() const

protected:
	std::chrono::nanosecond m_t0;
	std::chrono::nanosecond m_alarm_dt;
};
