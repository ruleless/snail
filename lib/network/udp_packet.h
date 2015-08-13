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

#ifndef KBE_SOCKETUDPPACKET_H
#define KBE_SOCKETUDPPACKET_H
	
#include "network/packet.h"
#include "common/ObjectPool.h"
	
namespace KBEngine{
namespace Network
{
class UDPPacket : public Packet
{
public:
	static ObjectPool<UDPPacket>& ObjPool();
	static void destroyObjPool();
	static size_t maxBufferSize();

    UDPPacket(MessageID msgID = 0, size_t res = 0);
	virtual ~UDPPacket(void);
	
	int recvFromEndPoint(EndPoint & ep, Address* pAddr = NULL);

	virtual void onReclaimObject();
};

typedef SmartPointer<UDPPacket> UDPPacketPtr;
}
}

#endif // KBE_SOCKETUDPPACKET_H
