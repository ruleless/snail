#include "PacketSender.h"
#include "network/Address.h"
#include "network/Bundle.h"
#include "network/Channel.h"
#include "network/EndPoint.h"
#include "network/EventDispatcher.h"
#include "network/NetworkManager.h"
#include "network/EventPoller.h"

PacketSender::PacketSender() 
		:mpEndpoint(NULL)
		,mpNetworkManager(NULL)
{
}

PacketSender::PacketSender(EndPoint &endpoint, NetworkManager &networkMgr)
		:mpEndpoint(&endpoint)
		,mpNetworkManager(&networkMgr)
{
}

PacketSender::~PacketSender()
{
}

int PacketSender::handleOutputNotification(int fd)
{
	processSend(NULL);
	return 0;
}

EReason PacketSender::processPacket(Channel* pChannel, Packet* pPacket)
{
	if (pChannel != NULL)
	{
		if (pChannel->getFilter())
		{
			return pChannel->getFilter()->send(pChannel, *this, pPacket);
		}
	}

	return this->processFilterPacket(pChannel, pPacket);
}

EReason PacketSender::checkSocketErrors(const EndPoint * pEndpoint)
{
	int err;
	EReason reason;

#ifdef unix
	err = errno;

	switch (err)
	{
	case ECONNREFUSED:	reason = Reason_NoSuchPort; break;
	case EAGAIN:		reason = Reason_ResourceUnavailable; break;
	case EPIPE:			reason = Reason_ClientDisconnected; break;
	case ECONNRESET:	reason = Reason_ClientDisconnected; break;
	case ENOBUFS:		reason = Reason_TransmitQueueFull; break;
	default:			reason = Reason_GeneralNetwork; break;
	}
#else
	err = WSAGetLastError();

	if (err == WSAEWOULDBLOCK || err == WSAEINTR)
	{
		reason = Reason_ResourceUnavailable;
	}
	else
	{
		switch (err)
		{
		case WSAECONNREFUSED:
			reason = Reason_NoSuchPort; 
			break;
		case WSAECONNRESET:
			reason = Reason_ClientDisconnected; 
			break;
		case WSAECONNABORTED:
			reason = Reason_ClientDisconnected; 
			break;
		default:
			reason = Reason_GeneralNetwork;
			break;
		}
	}
#endif

	return reason;
}

Channel* PacketSender::getChannel()
{
	return mpNetworkManager->findChannel(mpEndpoint->addr());
}
