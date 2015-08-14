#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "common/common.h"
#include "common/Timer.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/NetworkDef.h"
#include "network/Packet.h"
#include "network/Channel.h"

class EndPoint;
class Address;
class NetworkManager;
class EventDispatcher;

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
