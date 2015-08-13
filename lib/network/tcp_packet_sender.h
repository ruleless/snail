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


#ifndef KBE_NETWORKTCPPACKET_SENDER_H
#define KBE_NETWORKTCPPACKET_SENDER_H

#include "common/common.h"
#include "common/timer.h"
#include "common/ObjectPool.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/tcp_packet.h"
#include "network/packet_sender.h"

namespace KBEngine { 
namespace Network
{
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
	
	TCPPacketSender():PacketSender(){}
	TCPPacketSender(EndPoint & endpoint, NetworkManager & networkInterface);
	~TCPPacketSender();

	virtual void onGetError(Channel* pChannel);
	virtual bool processSend(Channel* pChannel);

protected:
	virtual Reason processFilterPacket(Channel* pChannel, Packet * pPacket);
};
}
}

#ifdef _INLINE
#include "tcp_packet_sender.inl"
#endif
#endif // KBE_NETWORKTCPPACKET_SENDER_H
