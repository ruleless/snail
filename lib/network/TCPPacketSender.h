#ifndef __TCPPACKETSENDER_H__
#define __TCPPACKETSENDER_H__

#include "common/common.h"
#include "common/Timer.h"
#include "common/ObjectPool.h"
#include "helper/debug_helper.h"
#include "network/NetworkDef.h"
#include "network/Network.h"
#include "network/TCPPacket.h"
#include "network/PacketSender.h"

class EndPoint;
class Channel;
class Address;
class NetworkManager;
class EventDispatcher;

class TCPPacketSender : public PacketSender
{
public:
	static ObjectPool<TCPPacketSender>& ObjPool();
	static void destroyObjPool();
	
	TCPPacketSender() : PacketSender(){}
	TCPPacketSender(EndPoint &endpoint, NetworkManager &networkInterface);
	~TCPPacketSender();

	virtual void onGetError(Channel* pChannel);
	virtual bool processSend(Channel* pChannel);
protected:
	virtual EReason processFilterPacket(Channel* pChannel, Packet * pPacket);
};

#endif // __TCPPACKETSENDER_H__
