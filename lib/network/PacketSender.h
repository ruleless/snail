#ifndef __PACKETSENDER_H__
#define __PACKETSENDER_H__

#include "common/common.h"
#include "common/ObjectPool.h"
#include "network/Network.h"
#include "network/NetworkDef.h"

class Packet;
class EndPoint;
class Channel;
class Address;
class NetworkManager;
class EventDispatcher;

class PacketSender : public OutputNotificationHandler, public PoolObject
{
public:
	PacketSender();
	PacketSender(EndPoint &endpoint, NetworkManager &networkInterface);
	virtual ~PacketSender();

	virtual void onReclaimObject()
	{
		mpEndpoint = NULL;
		mpNetworkManager = NULL;
	}

	void setEndPoint(EndPoint* pEndpoint)
	{ 
		mpEndpoint = pEndpoint; 
	}
	EndPoint* getEndPoint() const
	{ 
		return mpEndpoint; 
	}

	virtual int handleOutputNotification(int fd);

	virtual EReason processPacket(Channel *pChannel, Packet *pPacket);
	virtual EReason processFilterPacket(Channel *pChannel, Packet *pPacket) = 0;

	static EReason checkSocketErrors(const EndPoint * pEndpoint);

	virtual Channel* getChannel();

	virtual bool processSend(Channel* pChannel) = 0;
protected:
	EndPoint* mpEndpoint;
	NetworkManager* mpNetworkManager;
};

#endif // __PACKETSENDER_H__
