#ifndef __UDPPACKET_H__
#define __UDPPACKET_H__

#include "common/ObjectPool.h"
#include "network/NetworkDef.h"
#include "network/Packet.h"

class UDPPacket : public Packet
{
  public:
	static ObjectPool<UDPPacket>& ObjPool();
	static void destroyObjPool();
	static size_t maxBufferSize();

	virtual void onReclaimObject();

    UDPPacket(MessageID msgID = 0, size_t res = 0);
	virtual ~UDPPacket(void);
	
	int recvFromEndPoint(EndPoint &ep, Address* pAddr = NULL);
};

#endif // __UDPPACKET_H__
