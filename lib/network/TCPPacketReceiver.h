#ifndef __TCPPACKETRECEIVER_H__
#define __TCPPACKETRECEIVER_H__

#include "common/common.h"
#include "common/Timer.h"
#include "common/ObjectPool.h"
#include "network/NetworkDef.h"
#include "network/Network.h"
#include "network/TCPPacket.h"
#include "network/PacketReceiver.h"

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
