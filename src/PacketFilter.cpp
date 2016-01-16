#include "PacketFilter.h"
#include "Channel.h"
#include "PacketReceiver.h"
#include "PacketSender.h"

EReason PacketFilter::send(Channel *pChannel, PacketSender &sender, Packet *pPacket)
{
	return sender.processFilterPacket(pChannel, pPacket);
}

EReason PacketFilter::recv(Channel *pChannel, PacketReceiver &receiver, Packet *pPacket)
{
	return receiver.processFilteredPacket(pChannel, pPacket);
}
