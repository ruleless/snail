#include "UDPPacket.h"
#include "network/Bundle.h"
#include "network/EndPoint.h"
#include "network/NetworkManager.h"
#include "network/MessageHandler.h"

static ObjectPool<UDPPacket> s_ObjPool("UDPPacket");
ObjectPool<UDPPacket>& UDPPacket::ObjPool()
{
	return s_ObjPool;
}

void UDPPacket::destroyObjPool()
{
// 	DEBUG_MSG(fmt::format("UDPPacket::destroyObjPool(): size {}.\n", 
// 		s_ObjPool.size()));

	s_ObjPool.destroy();
}

UDPPacket::UDPPacket(MessageID msgID, size_t res)
:Packet(msgID, false, res)
{
	data_resize(maxBufferSize());
	wpos(0);
}

UDPPacket::~UDPPacket(void)
{
}

size_t UDPPacket::maxBufferSize()
{
	return PACKET_MAX_SIZE_UDP;
}

void UDPPacket::onReclaimObject()
{
	Packet::onReclaimObject();
	data_resize(maxBufferSize());
}

int UDPPacket::recvFromEndPoint(EndPoint &ep, Address *pAddr)
{
	Assert(maxBufferSize() > wpos());

	int len = ep.recvfrom(data() + wpos(), size() - wpos(),	(u_int16_t *)&pAddr->port, (u_int32_t *)&pAddr->ip);

	if(len > 0)
		wpos(wpos() + len);

	return len;
}
