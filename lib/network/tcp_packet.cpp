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


#include "tcp_packet.h"
#ifndef _INLINE
#include "tcp_packet.inl"
#endif
#include "network/bundle.h"
#include "network/EndPoint.h"
#include "network/network_interface.h"
#include "network/message_handler.h"

namespace KBEngine { 
namespace Network
{
//-------------------------------------------------------------------------------------
static ObjectPool<TCPPacket> s_ObjPool("TCPPacket");
ObjectPool<TCPPacket>& TCPPacket::ObjPool()
{
	return s_ObjPool;
}

//-------------------------------------------------------------------------------------
void TCPPacket::destroyObjPool()
{
	DEBUG_MSG(fmt::format("TCPPacket::destroyObjPool(): size {}.\n", 
		s_ObjPool.size()));

	s_ObjPool.destroy();
}

//-------------------------------------------------------------------------------------
TCPPacket::TCPPacket(MessageID msgID, size_t res):
Packet(msgID, true, res)
{
	data_resize(maxBufferSize());
	wpos(0);
}

//-------------------------------------------------------------------------------------
TCPPacket::~TCPPacket(void)
{
}

//-------------------------------------------------------------------------------------
size_t TCPPacket::maxBufferSize()
{
	return PACKET_MAX_SIZE_TCP;
}

//-------------------------------------------------------------------------------------
void TCPPacket::onReclaimObject()
{
	Packet::onReclaimObject();
	data_resize(maxBufferSize());
}

//-------------------------------------------------------------------------------------
int TCPPacket::recvFromEndPoint(EndPoint & ep, Address* pAddr)
{
	//Assert(MessageHandlers::pMainMessageHandlers != NULL && "Must set up a MainMessageHandlers!\n");
	Assert(maxBufferSize() > wpos());
	int len = ep.recv(data() + wpos(), size() - wpos());
	
	if(len > 0) 
	{
		wpos(wpos() + len);

		// ע��:�����ڴ���0��ʱ�����DEBUG_MSG���ᵼ��WSAGetLastError����0�Ӷ�������ѭ��
		// DEBUG_MSG(fmt::format("TCPPacket::recvFromEndPoint: datasize={}, wpos={}.\n", len, wpos()));
	}

	return len; 
}

//-------------------------------------------------------------------------------------
}
}
