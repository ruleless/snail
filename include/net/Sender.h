#ifndef __SENDER_H__
#define __SENDER_H__

#include "NetDef.h"
#include "NetManager.h"

class Sender : public Thread
{
  public:
    Sender(NetManager *netMgr);
    virtual ~Sender();

	bool create();
	void close();

	virtual void run();
  private:
	NetManager *mNetMgr;
	bool mbRun;
};

#endif
