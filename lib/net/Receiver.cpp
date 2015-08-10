#include "Receiver.h"
#include "NetManager.h"

Receiver::Receiver(NetManager *netMgr)
		:mNetMgr(netMgr)
		,mbRun(false)
{
}

Receiver::~Receiver()
{
}

bool Receiver::create()
{
	if (!mbRun)
		return true;
	
	mbRun = true;
	start();
	return true;
}

void Receiver::close()
{
	if (mbRun)
	{
		mbRun = false;
		wait(NULL);
	}
}

void Receiver::run()
{
	while (mbRun)
	{		
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000*500;
		
		fd_set readSet;
		int maxFd = 0;
		bool bHasSet = mNetMgr->getFdSet(&readSet, &maxFd);
		int nready = 0;
		if (bHasSet)
		{
			nready = select(maxFd+1, &readSet, NULL, NULL, &timeout);
		}
		else
		{
			select(0, NULL, NULL, NULL, &timeout);
		}

		for (int fd = 0; nready > 0 && fd <= maxFd; ++fd)
		{
			if (FD_ISSET(fd, &readSet))
			{
				mNetMgr->_fillInputBuffer(fd);
				--nready;
			}
		}
	}
}
