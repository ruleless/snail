#include "net/Sender.h"

Sender::Sender(NetManager *netMgr)
		:mNetMgr(netMgr)
		,mbRun(false)
{
}

Sender::~Sender()
{
}

bool Sender::create()
{
	if (mbRun)
		return true;

	mbRun = true;
	start();
	return true;
}

void Sender::close()
{
	if (mbRun)
	{
		mbRun = false;
		wait(NULL);
	}
}

void Sender::run()
{
	while (mbRun)
	{
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 500*1000;

		fd_set writeSet;
		int maxFd = -1;
		bool bHasSet = mNetMgr->getFdSet(&writeSet, &maxFd);
		int nready = 0;
		if (bHasSet)
		{
			nready = select(maxFd+1, NULL, &writeSet, NULL, &timeout);
		}
		else
		{
			select(0, NULL, NULL, NULL, &timeout);
		}

		for (int fd = 0; nready > 0 && fd < maxFd; ++fd)
		{
			if (FD_ISSET(fd, &writeSet))
			{
				mNetMgr->_processOutput(fd);
				--nready;
			}
		}
	}
}
