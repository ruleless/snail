#ifndef __TCPPACKETRECEIVER_H__
#define __TCPPACKETRECEIVER_H__

#include "common.h"
#include "ObjectPool.h"
#include "Network.h"
#include "NetworkDef.h"
#include "TCPPacket.h"
#include "PacketReceiver.h"

class EndPoint;
class Channel;
class Address;
class NetworkManager;
class EventDispatcher;

class TCPPacketReceiver : public PacketReceiver
{
public:
	static ObjectPool<TCPPacketReceiver>& ObjPool();
	static void destroyObjPool();
	
	TCPPacketReceiver():PacketReceiver(){}
	TCPPacketReceiver(EndPoint &endpoint, NetworkManager &networkMgr);
	~TCPPacketReceiver();

	EReason processFilteredPacket(Channel *pChannel, Packet *pPacket);
protected:
	virtual bool processRecv(bool expectingPacket);
	PacketReceiver::ERecvState checkSocketErrors(int len, bool expectingPacket);

	virtual void onGetError(Channel *pChannel);
};

#endif // __TCPPACKETRECEIVER_H__
