#include "PacketReceiver.h"
#include "network/Address.h"
#include "network/bundle.h"
#include "network/Channel.h"
#include "network/EndPoint.h"
#include "network/EventDispatcher.h"
#include "network/NetworkManager.h"
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

		if (pChannel->getFilter())
		{
			return pChannel->getFilter()->recv(pChannel, *this, pPacket);
		}
	}

	return this->processFilteredPacket(pChannel, pPacket);
}

Channel* PacketReceiver::getChannel()
{
	return mpNetworkManager->findChannel(mpEndpoint->addr());
}
