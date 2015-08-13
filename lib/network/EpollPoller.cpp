#include "EpollPoller.h"
#include "helper/profile.h"

#ifdef HAS_EPOLL

#include <sys/epoll.h>

static ProfileVal g_idleProfile("Idle");

EpollPoller::EpollPoller(int expectedSize)
{
	mEpfd = epoll_create(expectedSize);
	if (mEpfd == -1)
	{
		ERROR_MSG(fmt::format("EpollPoller::EpollPoller: epoll_create failed: {}\n",
				kbe_strerror()));
	}
}

EpollPoller::~EpollPoller()
{
	if (mEpfd != -1)
	{
		close(mEpfd);
	}
}

int EpollPoller::processPendingEvents(double maxWait)
{
	const int MAX_EVENTS = 10;
	struct epoll_event events[ MAX_EVENTS ];
	int maxWaitInMilliseconds = int(ceil(maxWait * 1000));

#if ENABLE_WATCHERS
	g_idleProfile.start();
#else
	uint64 startTime = timestamp();
#endif

	KBEConcurrency::onStartMainThreadIdling();
	int nfds = epoll_wait(mEpfd, events, MAX_EVENTS, maxWaitInMilliseconds);
	KBEConcurrency::onEndMainThreadIdling();


#if ENABLE_WATCHERS
	g_idleProfile.stop();
	mSpareTime += g_idleProfile.lastTime_;
#else
	mSpareTime += timestamp() - startTime;
#endif

	for (int i = 0; i < nfds; ++i)
	{
		if (events[i].events & (EPOLLERR|EPOLLHUP))
		{
			this->triggerError(events[i].data.fd);
		}
		else
		{
			if (events[i].events & EPOLLIN)
			{
				this->triggerRead(events[i].data.fd);
			}

			if (events[i].events & EPOLLOUT)
			{
				this->triggerWrite(events[i].data.fd);
			}
		}
	}

	return nfds;
}

bool EpollPoller::doRegister(int fd, bool isRead, bool isRegister)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.data.fd = fd;

	int op;
	if (this->isRegistered(fd, !isRead))
	{
		op = EPOLL_CTL_MOD;

		ev.events = isRegister ? EPOLLIN|EPOLLOUT :	isRead ? EPOLLOUT : EPOLLIN;
	}
	else
	{
		ev.events = isRead ? EPOLLIN : EPOLLOUT;
		op = isRegister ? EPOLL_CTL_ADD : EPOLL_CTL_DEL;
	}

	if (epoll_ctl(mEpfd, op, fd, &ev) < 0)
	{
		const char* MESSAGE = "EpollPoller::doRegister: Failed to {} {} file "
			"descriptor {} ({})\n";
		if (errno == EBADF)
		{
			WARNING_MSG(fmt::format(MESSAGE,
				(isRegister ? "add" : "remove"),
				(isRead ? "read" : "write"),
				fd,
				kbe_strerror()));
		}
		else
		{
			ERROR_MSG(fmt::format(MESSAGE,
				(isRegister ? "add" : "remove"),
				(isRead ? "read" : "write"),
				fd,
				kbe_strerror()));
		}

		return false;
	}

	return true;
}

#endif // HAS_EPOLL
