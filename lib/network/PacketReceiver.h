#ifndef __PACKETRECEIVER_H__
#define __PACKETRECEIVER_H__

#include "common/common.h"
#include "common/ObjectPool.h"
#include "common/timer.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/TCPPacket.h"

class EndPoint;
class Channel;
class Address;
class NetworkManager;
class EventDispatcher;

class PacketReceiver : public InputNotificationHandler, public PoolObject
{
public:
	enum ERecvState
	{
		RecvState_Interrupt = -1,
		RecvState_Break,
		RecvState_Continue,
	};

	enum EPacketReceiverType
	{
		ReceiverType_TCP = 0,
		ReceiverType_UDP,
	};

	PacketReceiver();
	PacketReceiver(EndPoint &endpoint, NetworkManager &networkMgr);
	virtual ~PacketReceiver();

	virtual void onReclaimObject()
	{
		mpEndpoint = NULL;
		mpNetworkManager = NULL;
	}

	virtual PacketReceiver::EPacketReceiverType type() const
	{
		return ReceiverType_TCP;
	}

	void setEndPoint(EndPoint* pEndpoint)
	{ 
		mpEndpoint = pEndpoint; 
	}

	EndPoint* getEndPoint() const
	{
		return mpEndpoint; 
	}

	virtual int handleInputNotification(int fd);

	virtual Reason processPacket(Channel *pChannel, Packet *pPacket);
	virtual Reason processFilteredPacket(Channel *pChannel, Packet *pPacket) = 0;

	virtual Channel* getChannel();
protected:
	virtual bool processRecv(bool expectingPacket) = 0;
	virtual ERecvState checkSocketErrors(int len, bool expectingPacket) = 0;
protected:
	EndPoint *mpEndpoint;
	NetworkManager *mpNetworkManager;
};

#endif // __PACKETRECEIVER_H__
