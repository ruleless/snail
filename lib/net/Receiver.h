#ifndef __RECEIVER_H__
#define __RECEIVER_H__

#include "NetDef.h"

class NetManager;
class Receiver : public Thread
{
  public:
    Receiver(NetManager *netMgr);
    virtual ~Receiver();

	bool create();
	void close();

	virtual void run();
  private:
	NetManager *mNetMgr;
	bool mbRun;
};

#endif
