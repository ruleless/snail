#include "PacketFilter.h"
#include "network/Channel.h"
#include "network/NetworkManager.h"
#include "network/PacketReceiver.h"
#include "network/PacketSender.h"

EReason PacketFilter::send(Channel *pChannel, PacketSender &sender, Packet *pPacket)
{
	return sender.processFilterPacket(pChannel, pPacket);
}

EReason PacketFilter::recv(Channel *pChannel, PacketReceiver &receiver, Packet *pPacket)
{
	return receiver.processFilteredPacket(pChannel, pPacket);
}