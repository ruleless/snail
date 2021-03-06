#ifndef __ACCEPTER_H__
#define __ACCEPTER_H__

#include "NetDef.h"

class NetManager;
class Accepter : public Thread
{
  public:
    Accepter(NetManager *netMgr, int listenPort = 60000, int listenQ = 5);
    virtual ~Accepter();

	bool create();
	void close();
	virtual void run();
  private:
	NetManager *mNetMgr;
	bool mbRun;
	int mSockFd;
	int mListenPort;
	int mListenQ;
};

#endif
