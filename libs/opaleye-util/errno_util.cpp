#include "errno_util.hpp"

#include <cstring>
#include <errno.h>

errno_util::errno_util() : m_locale((locale_t)0)
{

}
errno_util::~errno_util()
{
	if(m_locale)
	{
		freelocale(m_locale);
		m_locale = (locale_t)0;
	}
}

const char* errno_util::to_str(int * out_inner_errno)
{
	if( ! m_locale )
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if( ! m_locale )
		{
			m_locale = newlocale(LC_ALL_MASK, "C", (locale_t)0);
			if( ! m_locale )
			{
				return "newlocale failed";
			}
		}
	}

	const int err = errno;
	errno = 0;
	char* msg = strerror_l(err, m_locale.load());
	if(errno != 0)
	{
		if(out_inner_errno)
		{
			*out_inner_errno = errno;
		}
	}

	return msg;
}
