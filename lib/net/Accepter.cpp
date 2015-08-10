#include "Accepter.h"
#include "NetManager.h"

Accepter::Accepter(NetManager *netMgr, int listenPort, int listenQ)
		:mNetMgr(netMgr)
		,mListenPort(listenPort)
		,mListenQ(listenQ)
		,mSockFd(-1)
		,mbRun(false)
{
}

Accepter::~Accepter()
{
}

bool Accepter::create()
{
	if (mbRun)
		return true;

	mSockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (mSockFd < 0)
		return false;

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(mListenPort);

	if (bind(mSockFd, (const SA *)&addr, sizeof(addr)))
	{
		::close(mSockFd);
		return false;
	}

	if (listen(mSockFd, mListenQ) < 0)
	{
		::close(mSockFd);
		return false;
	}

	mbRun = true;
	start();
	return true;
}

void Accepter::close()
{
	if (mbRun)
	{
		::close(mSockFd);
		mbRun = false;
		wait(NULL);
	}
}

void Accepter::run()
{
	while (mbRun)
	{
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 500*1000;

		fd_set set;
		FD_SET(mSockFd, &set);
		
		int nready = select(mSockFd+1, &set, NULL, NULL, &timeout);

		if (nready > 0)
		{
			struct sockaddr_in clientAddr;
			socklen_t addrLen;
			int connFd = accept(mSockFd, (SA *)&clientAddr, &addrLen);

			if (connFd >= 0)
			{
				NewConnInfo info;
				info.fd = connFd;
				memcpy(&info.addr, &clientAddr, sizeof(clientAddr));
				info.addrLen = addrLen;

				mNetMgr->_pushNewConn(&info);
			}
		}
	}
}
