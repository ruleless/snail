#ifndef __UDPPACKETRECEIVER_H__
#define __UDPPACKETRECEIVER_H__

#include "common.h"
#include "ObjectPool.h"
#include "Network.h"
#include "NetworkDef.h"
#include "UDPPacket.h"
#include "PacketReceiver.h"

class Channel;
class Address;
class NetworkManager;
class EventDispatcher;

class UDPPacketReceiver : public PacketReceiver
{
public:
	static ObjectPool<UDPPacketReceiver>& ObjPool();
	static void destroyObjPool();

	UDPPacketReceiver():PacketReceiver(){}
	UDPPacketReceiver(EndPoint &endpoint, NetworkManager &networkMgr);
	~UDPPacketReceiver();

	EReason processFilteredPacket(Channel *pChannel, Packet *pPacket);
	
	virtual PacketReceiver::EPacketReceiverType type() const
	{
		return ReceiverType_UDP;
	}
protected:
	virtual bool processRecv(bool expectingPacket);
	PacketReceiver::ERecvState checkSocketErrors(int len, bool expectingPacket);
};

#endif // __UDPPACKETRECEIVER_H__
