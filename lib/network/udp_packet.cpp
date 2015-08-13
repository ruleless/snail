/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "udp_packet.h"
#ifndef _INLINE
#include "udp_packet.inl"
#endif
#include "network/bundle.h"
#include "network/EndPoint.h"
#include "network/network_interface.h"
#include "network/message_handler.h"

namespace KBEngine { 
namespace Network
{
//-------------------------------------------------------------------------------------
static ObjectPool<UDPPacket> s_ObjPool("UDPPacket");
ObjectPool<UDPPacket>& UDPPacket::ObjPool()
{
	return s_ObjPool;
}

//-------------------------------------------------------------------------------------
void UDPPacket::destroyObjPool()
{
	DEBUG_MSG(fmt::format("UDPPacket::destroyObjPool(): size {}.\n", 
		s_ObjPool.size()));

	s_ObjPool.destroy();
}

//-------------------------------------------------------------------------------------
UDPPacket::UDPPacket(MessageID msgID, size_t res):
Packet(msgID, false, res)
{
	data_resize(maxBufferSize());
	wpos(0);
}

//-------------------------------------------------------------------------------------
UDPPacket::~UDPPacket(void)
{
}

//-------------------------------------------------------------------------------------
size_t UDPPacket::maxBufferSize()
{
	return PACKET_MAX_SIZE_UDP;
}

//-------------------------------------------------------------------------------------
void UDPPacket::onReclaimObject()
{
	Packet::onReclaimObject();
	data_resize(maxBufferSize());
}

//-------------------------------------------------------------------------------------
int UDPPacket::recvFromEndPoint(EndPoint & ep, Address* pAddr)
{
	Assert(maxBufferSize() > wpos());

	// ���������Ĵ�С���ڽ��ջ�������ʱ��recvfrom����-1
	int len = ep.recvfrom(data() + wpos(), size() - wpos(),
		(u_int16_t*)&pAddr->port, (u_int32_t*)&pAddr->ip);

	if(len > 0)
		wpos(wpos() + len);

	return len;
}

//-------------------------------------------------------------------------------------
}
}
