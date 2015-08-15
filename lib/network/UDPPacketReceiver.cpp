#include "UDPPacketReceiver.h"
#include "network/Address.h"
#include "network/Bundle.h"
#include "network/Channel.h"
#include "network/EndPoint.h"
#include "network/EventDispatcher.h"
#include "network/NetworkManager.h"
#include "network/EventPoller.h"
#include "network/error_reporter.h"

static ObjectPool<UDPPacketReceiver> s_ObjPool("UDPPacketReceiver");
ObjectPool<UDPPacketReceiver>& UDPPacketReceiver::ObjPool()
{
	return s_ObjPool;
}

void UDPPacketReceiver::destroyObjPool()
{
	DEBUG_MSG(fmt::format("UDPPacketReceiver::destroyObjPool(): size {}.\n", 
		s_ObjPool.size()));

	s_ObjPool.destroy();
}

UDPPacketReceiver::UDPPacketReceiver(EndPoint & endpoint, NetworkManager &networkMgr) 
:PacketReceiver(endpoint, networkMgr)
{
}

UDPPacketReceiver::~UDPPacketReceiver()
{
}

bool UDPPacketReceiver::processRecv(bool expectingPacket)
{	
	Address	srcAddr;
	UDPPacket *pChannelReceiveWindow = UDPPacket::ObjPool().createObject();
	int len = pChannelReceiveWindow->recvFromEndPoint(*mpEndpoint, &srcAddr);

	if (len <= 0)
	{
		UDPPacket::ObjPool().reclaimObject(pChannelReceiveWindow);
		PacketReceiver::ERecvState rstate = this->checkSocketErrors(len, expectingPacket);
		return rstate == PacketReceiver::RecvState_Continue;
	}
	
	Channel *pSrcChannel = mpNetworkManager->findChannel(srcAddr);

	if(pSrcChannel == NULL) 
	{
		EndPoint* pNewEndPoint = EndPoint::ObjPool().createObject();
		pNewEndPoint->addr(srcAddr.port, srcAddr.ip);

		pSrcChannel = Channel::ObjPool().createObject();
		bool ret = pSrcChannel->initialize(*mpNetworkManager, pNewEndPoint, Channel::External, Protocol_UDP);
		if(!ret)
		{
			ERROR_MSG(fmt::format("UDPPacketReceiver::processRecv: initialize({}) is failed!\n",
				pSrcChannel->c_str()));

			pSrcChannel->destroy();
			Channel::ObjPool().reclaimObject(pSrcChannel);
			UDPPacket::ObjPool().reclaimObject(pChannelReceiveWindow);
			return false;
		}

		if(!mpNetworkManager->registerChannel(pSrcChannel))
		{
			ERROR_MSG(fmt::format("UDPPacketReceiver::processRecv: registerChannel({}) is failed!\n",
				pSrcChannel->c_str()));

			UDPPacket::ObjPool().reclaimObject(pChannelReceiveWindow);
			pSrcChannel->destroy();
			Channel::ObjPool().reclaimObject(pSrcChannel);
			return false;
		}
	}
	
	Assert(pSrcChannel != NULL);

	if(pSrcChannel->isCondemn())
	{
		UDPPacket::ObjPool().reclaimObject(pChannelReceiveWindow);
		mpNetworkManager->deregisterChannel(pSrcChannel);
		pSrcChannel->destroy();
		Channel::ObjPool().reclaimObject(pSrcChannel);
		return false;
	}

	EReason ret = this->processPacket(pSrcChannel, pChannelReceiveWindow);

	if(ret != Reason_Success)
		;
	
	return true;
}

EReason UDPPacketReceiver::processFilteredPacket(Channel *pChannel, Packet *pPacket)
{
	if(pPacket)
	{
		pChannel->addReceiveWindow(pPacket);
	}

	return Reason_Success;
}

PacketReceiver::ERecvState UDPPacketReceiver::checkSocketErrors(int len, bool expectingPacket)
{
	if (len == 0)
	{
		WARNING_MSG(fmt::format("PacketReceiver::processPendingEvents: "
			"Throwing REASON_GENERAL_NETWORK (1)- {}\n",
			strerror( errno )));

		return RecvState_Continue;
	}
	
#ifdef _WIN32
	DWORD wsaErr = WSAGetLastError();
#endif //def _WIN32

	if (
#ifdef _WIN32
		wsaErr == WSAEWOULDBLOCK && !expectingPacket
#else
		errno == EAGAIN && !expectingPacket
#endif
		)
	{
		return RecvState_Break;
	}

#ifdef unix
	if (errno == EAGAIN ||
		errno == ECONNREFUSED ||
		errno == EHOSTUNREACH)
	{
		Address offender;

		if (mpEndpoint->getClosedPort(offender))
		{
			// If we got a NO_SUCH_PORT error and there is an internal
			// channel to this address, mark it as remote failed.  The logic
			// for dropping external channels that get NO_SUCH_PORT
			// exceptions is built into BaseApp::onClientNoSuchPort().
			if (errno == ECONNREFUSED)
			{
				// Œ¥ µœ÷
			}

			return RecvState_Continue;
		}
		else
		{
			WARNING_MSG("UDPPacketReceiver::processPendingEvents: "
				"getClosedPort() failed\n");
		}
	}
#else
	if (wsaErr == WSAECONNRESET)
	{
		return RecvState_Continue;
	}
#endif // unix

#ifdef _WIN32
	WARNING_MSG(fmt::format("UDPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - {}\n",
				wsaErr));
#else
	WARNING_MSG(fmt::format("UDPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - {}\n",
			kbe_strerror()));
#endif	

	return RecvState_Continue;
}
