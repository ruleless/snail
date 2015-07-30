#ifndef __ACCEPTER_H__
#define __ACCEPTER_H__

#include "NetDef.h"
#include "NetManager"

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
	int mListenPort;
	int mListenQ;
	int mSockFd;
};

#endif
