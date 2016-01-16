#ifndef __PACKETFILTER_H__
#define __PACKETFILTER_H__

#include "NetworkDef.h"
#include "SmartPointer.h"
#include "RefCountable.h"

class Channel;
class NetworkManager;
class Packet;
class Address;
class PacketReceiver;
class PacketSender;

class PacketFilter : public RefCountable
{
  public:
	virtual ~PacketFilter() {}

	virtual EReason send(Channel *pChannel, PacketSender &sender, Packet *pPacket);

	virtual EReason recv(Channel *pChannel, PacketReceiver &receiver, Packet *pPacket);
};

typedef SmartPointer<PacketFilter> PacketFilterPtr;

#endif // __PACKETFILTER_H__
