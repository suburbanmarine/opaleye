#pragma once

#include <locale.h>

#include <atomic>
#include <mutex>

class errno_util
{
public:
	errno_util();
	~errno_util();

	const char* to_str(int * out_inner_errno = nullptr);

protected:
	std::atomic<locale_t> m_locale;
	std::mutex m_mutex;
};
