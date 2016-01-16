#include "PacketReceiver.h"
#include "Channel.h"
#include "EndPoint.h"
#include "EventDispatcher.h"
#include "NetworkManager.h"

PacketReceiver::PacketReceiver()
		:mpEndpoint(NULL)
		,mpNetworkManager(NULL)
{
}

PacketReceiver::PacketReceiver(EndPoint &endpoint, NetworkManager &networkMgr)
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

EReason PacketReceiver::processPacket(Channel *pChannel, Packet *pPacket)
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
