#ifndef __TCPPACKET_H__
#define __TCPPACKET_H__

#include "ObjectPool.h"
#include "NetworkDef.h"
#include "Packet.h"

class TCPPacket : public Packet
{
  public:
	static ObjectPool<TCPPacket>& ObjPool();
	static void destroyObjPool();

	static size_t maxBufferSize();

    TCPPacket(MessageID msgID = 0, size_t res = 0);
	virtual ~TCPPacket(void);
	
	int recvFromEndPoint(EndPoint & ep, Address* pAddr = NULL);

	virtual void onReclaimObject();
};

#endif // __TCPPACKET_H__
