#ifndef __UDPPACKETRECEIVER_H__
#define __UDPPACKETRECEIVER_H__

#include "common/common.h"
#include "common/timer.h"
#include "common/ObjectPool.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/UDPPacket.h"
#include "network/PacketReceiver.h"

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

	Reason processFilteredPacket(Channel *pChannel, Packet *pPacket);
	
	virtual PacketReceiver::EPacketReceiverType type() const
	{
		return ReceiverType_UDP;
	}
protected:
	virtual bool processRecv(bool expectingPacket);
	PacketReceiver::ERecvState checkSocketErrors(int len, bool expectingPacket);
};

#endif // __UDPPACKETRECEIVER_H__
