#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "common/common.h"
#include "network/Network.h"
#include "network/NetworkDef.h"
#include "network/EndPoint.h"
#include "network/Channel.h"

class NetworkManager;

class Listener : public InputNotificationHandler
{
  public:
	Listener(EndPoint &endpoint, Channel::ETraits traits, NetworkManager &networkMgr);
	~Listener();
  private:
	virtual int handleInputNotification(int fd);
  private:
	NetworkManager &mNetworkManager;
	EndPoint &mEndpoint;
	Channel::ETraits mTraits;
};

#endif // __LISTENER_H__
