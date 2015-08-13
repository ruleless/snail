#include "PacketReceiver.h"
#include "network/address.h"
#include "network/bundle.h"
#include "network/channel.h"
#include "network/EndPoint.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/EventPoller.h"

PacketReceiver::PacketReceiver()
:mpEndpoint(NULL)
,mpNetworkManager(NULL)
{
}

PacketReceiver::PacketReceiver(EndPoint &endpoint,NetworkManager &networkMgr)
:mpEndpoint(&endpoint)
,mpNetworkManager(&networkMgr)
{
}

PacketReceiver::~PacketReceiver()
{
}

int PacketReceiver::handleInputNotification(int fd)
{
	if (this->processRecv(true))
	{
		while (this->processRecv(false));
	}

	return 0;
}

Reason PacketReceiver::processPacket(Channel *pChannel, Packet *pPacket)
{
	if (pChannel != NULL)
	{
		pChannel->onPacketReceived(pPacket->length());

		if (pChannel->pFilter())
		{
			return pChannel->pFilter()->recv(pChannel, *this, pPacket);
		}
	}

	return this->processFilteredPacket(pChannel, pPacket);
}

Channel* PacketReceiver::getChannel()
{
	return mpNetworkManager->findChannel(mpEndpoint->addr());
}
