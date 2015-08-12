#include "EchoServer.h"

EchoServer::EchoServer()
{
}

EchoServer::~EchoServer()
{	
}

bool EchoServer::create()
{
	if (!mNetMgr.create())
		return false;
	mNetMgr.setListener(this);
	return true;
}

void EchoServer::run()
{
	for (;;)
	{
		mNetMgr.update();
	}	
}

void EchoServer::close()
{
	mNetMgr.setListener(NULL);
	mNetMgr.close();
}

void EchoServer::onRecvStream(SockInputStream *inputStream)
{
	static const int s_buffSize = 1024;
	char buff[s_buffSize+1];
	memset(buff, 0, sizeof(buff));
	int n = inputStream->batchRead(buff, s_buffSize);
	int sockFd = inputStream->getSockFd();
	
	if (mNetMgr.sendData(sockFd, buff, n) != n)
	{
		fprintf(stderr, "conn %d send err.\n", sockFd);
	}
	// printf("recv:%s", buff);
}


int main(int argc, char *argv[])
{
	EchoServer server;
	if (!server.create())
	{
		errQuit("create server failed.");
	}
	server.run();
	
	server.close();
	exit(0);
}
