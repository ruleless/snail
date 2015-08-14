#include "TCPPacket.h"
#include "network/bundle.h"
#include "network/EndPoint.h"
#include "network/NetworkManager.h"
#include "network/message_handler.h"

static ObjectPool<TCPPacket> s_ObjPool("TCPPacket");
ObjectPool<TCPPacket>& TCPPacket::ObjPool()
{
	return s_ObjPool;
}

void TCPPacket::destroyObjPool()
{
	DEBUG_MSG(fmt::format("TCPPacket::destroyObjPool(): size {}.\n", 
		s_ObjPool.size()));

	s_ObjPool.destroy();
}

TCPPacket::TCPPacket(MessageID msgID, size_t res)
:Packet(msgID, true, res)
{
	data_resize(maxBufferSize());
	wpos(0);
}

TCPPacket::~TCPPacket(void)
{
}

size_t TCPPacket::maxBufferSize()
{
	return PACKET_MAX_SIZE_TCP;
}

void TCPPacket::onReclaimObject()
{
	Packet::onReclaimObject();
	data_resize(maxBufferSize());
}

int TCPPacket::recvFromEndPoint(EndPoint &ep, Address *pAddr)
{
	Assert(maxBufferSize() > wpos());
	int len = ep.recv(data() + wpos(), size() - wpos());
	
	if(len > 0) 
	{
		wpos(wpos() + len);
	}

	return len; 
}
