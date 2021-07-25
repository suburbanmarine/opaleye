#pragma once

#include <chrono>

class Stopwatch
{
public:
	///Default to CLOCK_MONOTONIC
	Stopwatch();
	///Allow picking a different clock than CLOCK_MONOTONIC
	Stopwatch(clockid_t clk_id);
	~Stopwatch();

	/// reset epoch
	void reset();

	std::chrono::nanoseconds now() const;
	std::chrono::nanoseconds duration() const;

	bool is_expired() const;

	std::chrono::nanoseconds time_left() const;

	template<typename Rep, typename Period>
	void set_alarm_dt(const std::chrono::duration<Rep, Period>& dt)
	{
		m_alarm_dt = dt;
	}

	///set clockid after construction. Invalidates current alarm and epoch
	///call reset after calling this to refresh epoch
	void set_clockid(clockid_t clk_id)
	{
		m_clk_id = clk_id;
	}

protected:

	clockid_t m_clk_id;

	std::chrono::nanoseconds m_t0;
	std::chrono::nanoseconds m_alarm_dt;
};
