/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <clocale>

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
