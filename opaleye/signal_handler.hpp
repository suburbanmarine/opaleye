/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <atomic>

class Signal_handler
{
public:

	Signal_handler() : got_sigint(false)
	{

	}

	//mask all signals, for worker threads
	bool mask_all_signals();
	//mask def signals, for main thread
	bool mask_def_signals();
	//wait for signal
	bool wait_for_signal();

	bool has_sigint()
	{
		return got_sigint.exchange(false);
	}

protected:
	std::atomic<bool> got_sigint;
};