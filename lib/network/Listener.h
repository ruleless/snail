#ifndef __LISTENER_H__
#define __LISTENER_H__

#include "common/common.h"
#include "common/timer.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/Packet.h"
#include "network/channel.h"

class EndPoint;
class Address;
class NetworkManager;
class EventDispatcher;

class Listener : public InputNotificationHandler
{
public:
	Listener(EndPoint &endpoint, Channel::Traits traits, NetworkManager &networkMgr);
	~Listener();
private:
	virtual int handleInputNotification(int fd);
private:
	NetworkManager &mNetworkManager;
	EndPoint &mEndpoint;
	Channel::Traits mTraits;
};

#endif // __LISTENER_H__
