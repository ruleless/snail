#ifndef __STREAMSERVER_H__
#define __STREAMSERVER_H__

#include "common/common.h"
#include "net/NetDef.h"
#include "net/NetManager.h"

class EchoServer : public NetManager::Listener
{
  public:
	EchoServer();
	virtual ~EchoServer();

	bool create();
	void run();
	void close();

	virtual void onRecvStream(SockInputStream *inputStream);
  private:
	NetManager mNetMgr;
};

#endif
