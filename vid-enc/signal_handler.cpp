#include "signal_handler.hpp"

#include <spdlog/spdlog.h>

 #include <signal.h>

bool Signal_handler::mask_all_signals()
{
	sigset_t set;
	int ret = sigfillset(&set);

	ret = pthread_sigmask(SIG_SETMASK, &set, NULL);
	if(ret != 0)
	{
		SPDLOG_ERROR("Could not set signal mask");
		return false;
	}
	return true;
}

bool Signal_handler::mask_def_signals()
{
	sigset_t set;
	int ret = sigemptyset(&set);

	// posix timer
	sigaddset(&set, SIGALRM);
	//terminal hangup
	sigaddset(&set, SIGHUP);
	// ctrl-c
	sigaddset(&set, SIGINT);
	// ctrl-backslash
	// sigaddset(&set, SIGQUIT);
	//user 1
	sigaddset(&set, SIGUSR1);
	//user 2
	sigaddset(&set, SIGUSR2);

	for(int i = SIGRTMIN; i <= SIGRTMAX; i++)
	{
		sigaddset(&set, i);
	}

	ret = pthread_sigmask(SIG_SETMASK, &set, NULL);
	if(ret != 0)
	{
		SPDLOG_ERROR("Could not set signal mask");
		return false;
	}

	return true;
}

bool Signal_handler::wait_for_signal()
{
	sigset_t set;
	int ret = sigfillset(&set);

	bool keep_going = true;
	do
	{
		siginfo_t info;
		int ret = sigwaitinfo(&set, &info);
		if(ret < 0)
		{
			switch(errno)
			{
				case EAGAIN:
				{
					continue;
				}
				case EINTR:
				{
					continue;
				}
				case EINVAL:
				{
					return false;
				}
				default:
				{
					return false;
				}
			}
		}
		else
		{
			keep_going = false;
			switch(ret)
			{
				case SIGALRM:
				{
					break;
				}
				case SIGHUP:
				{
					break;
				}
				case SIGINT:
				{
					got_sigint = true;
					break;
				}
				case SIGQUIT:
				{
					break;
				}
				case SIGUSR1:
				{
					break;
				}
				case SIGUSR2:
				{
					break;
				}
			}
		}
	} while(keep_going);

	return true;
}