#ifndef __PACKETRECEIVER_H__
#define __PACKETRECEIVER_H__

#include "common.h"
#include "ObjectPool.h"
#include "NetworkDef.h"
#include "Network.h"

class EndPoint;
class Channel;
class NetworkManager;
class EventDispatcher;

class PacketReceiver : public InputNotificationHandler, public PoolObject
{
public:
	enum ERecvState
	{
		RecvState_Interrupt = -1,
		RecvState_Break = 0,
		RecvState_Continue = 1,
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

	virtual EReason processPacket(Channel *pChannel, Packet *pPacket);
	virtual EReason processFilteredPacket(Channel *pChannel, Packet *pPacket) = 0;

	virtual Channel* getChannel();
protected:
	virtual bool processRecv(bool expectingPacket) = 0;
	virtual ERecvState checkSocketErrors(int len, bool expectingPacket) = 0;
protected:
	EndPoint *mpEndpoint;
	NetworkManager *mpNetworkManager;
};

#endif // __PACKETRECEIVER_H__
