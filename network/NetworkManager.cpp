#include "NetworkManager.h"
#include "network/Address.h"
#include "network/EventDispatcher.h"
#include "network/PacketReceiver.h"
#include "network/Listener.h"
#include "network/Channel.h"
#include "network/Packet.h"
#include "network/DelayedChannels.h"
#include "network/Network.h"
#include "network/MessageHandler.h"

NetworkManager::NetworkManager(EventDispatcher *pDispatcher,
							   int32 minExtlisteningPort,
							   int32 maxExtlisteningPort, 
							   const char *extlisteningInterface,
							   uint32 extrbuffer, 
							   uint32 extwbuffer,
							   int32 intlisteningPort,
							   const char *intlisteningInterface,
							   uint32 intrbuffer, 
							   uint32 intwbuffer)
		:mpExtListenerReceiver(NULL)
		,mExtEndpoint()
		,mpIntListenerReceiver(NULL)
		,mIntEndpoint()
		,mChannels()
		,mpExtensionData(NULL)
		,mIsExternal(minExtlisteningPort != -1)
		,mNumExtChannels(0)
		,mpDelayedChannels(new DelayedChannels())
		,mpChannelTimeOutHandler(NULL)
		,mpChannelDeregisterHandler(NULL)
		,mpDispatcher(pDispatcher)
{
	if(isExternal())
	{
		mpExtListenerReceiver = new Listener(mExtEndpoint, Channel::External, *this);
		this->recreateListeningSocket("External", htons(minExtlisteningPort), htons(maxExtlisteningPort), 
									  extlisteningInterface, &mExtEndpoint, mpExtListenerReceiver, extrbuffer, extwbuffer);

		if(minExtlisteningPort != -1)
		{
			Assert(mExtEndpoint.good() && "Channel::External: no available port.\n");
		}
	}

	if(intlisteningPort != -1)
	{
		mpIntListenerReceiver = new Listener(mIntEndpoint, Channel::Internal, *this);
		this->recreateListeningSocket("INTERNAL", intlisteningPort, intlisteningPort, 
									  intlisteningInterface, &mIntEndpoint, mpIntListenerReceiver, intrbuffer, intwbuffer);
	}

	Assert(good() && "NetworkInterface::NetworkInterface: no available port.\n");

	mpDelayedChannels->initialize(this->dispatcher(), this);
}

NetworkManager::~NetworkManager()
{
	ChannelMap::iterator iter = mChannels.begin();
	while (iter != mChannels.end())
	{
		ChannelMap::iterator oldIter = iter++;
		Channel * pChannel = oldIter->second;
		pChannel->destroy();
		delete pChannel;
	}

	mChannels.clear();

	this->closeSocket();

	if (mpDispatcher != NULL)
	{
		mpDelayedChannels->finalise(this->dispatcher());
		mpDispatcher = NULL;
	}

	SafeDelete(mpDelayedChannels);
	SafeDelete(mpExtListenerReceiver);
	SafeDelete(mpIntListenerReceiver);
}

bool NetworkManager::recreateListeningSocket(const char *pEndPointName,
											 uint16 minListeningPort, 
											 uint16 maxListeningPort, 
											 const char *listeningInterface, 
											 EndPoint *pEndPoint,
											 Listener *pListener,
											 uint32 rbuffer, 
											 uint32 wbuffer)
{
	Assert(listeningInterface && pEndPoint && pListener);

	if (pEndPoint->good())
	{
		this->dispatcher().deregisterReadFileDescriptor(*pEndPoint);
		pEndPoint->close();
	}

	Address address;
	address.ip = 0;
	address.port = 0;

	pEndPoint->socket(SOCK_STREAM);
	if (!pEndPoint->good())
	{
		// 		ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): couldn't create a socket\n",
		// 			pEndPointName));

		return false;
	}
	
	this->dispatcher().registerReadFileDescriptor(*pEndPoint, pListener);
	
	u_int32_t ifIPAddr = INADDR_ANY;

	bool listeningInterfaceEmpty = (listeningInterface == NULL || listeningInterface[0] == 0);

	if(pEndPoint->findIndicatedInterface(listeningInterface, ifIPAddr) == 0)
	{
		char szIp[MAX_IP] = {0};
		Address::ip2string(ifIPAddr, szIp);

		// 		INFO_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
		// 				"Creating on interface '{}' (= {})\n",
		// 			pEndPointName, listeningInterface, szIp));
	}
	else if (!listeningInterfaceEmpty)
	{
		// 		WARNING_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
		// 				"Couldn't parse interface spec '{}' so using all interfaces\n",
		// 			pEndPointName, listeningInterface));
	}
	
	bool foundport = false;
	uint32 listeningPort = minListeningPort;
	if(minListeningPort != maxListeningPort)
	{
		for(int lpIdx = ntohs(minListeningPort); lpIdx < ntohs(maxListeningPort); ++lpIdx)
		{
			listeningPort = htons(lpIdx);
			if (pEndPoint->bind(listeningPort, ifIPAddr) != 0)
			{
				continue;
			}
			else
			{
				foundport = true;
				break;
			}
		}
	}
	else
	{
		if (pEndPoint->bind(listeningPort, ifIPAddr) == 0)
		{
			foundport = true;
		}
	}

	if(!foundport)
	{
		// 		ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
		// 				"Couldn't bind the socket to {}:{} ({})\n",
		// 			pEndPointName, inet_ntoa((struct in_addr&)ifIPAddr), ntohs(listeningPort), __strerror()));
		
		pEndPoint->close();
		return false;
	}

	pEndPoint->getlocaladdress((u_int16_t *)&address.port, (u_int32_t *)&address.ip);
	if (0 == address.ip)
	{
		u_int32_t addr;
		if(0 == pEndPoint->getDefaultInterfaceAddress(addr))
		{
			address.ip = addr;

			char szIp[MAX_IP] = {0};
			Address::ip2string(address.ip, szIp);
			// 			INFO_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
			// 					"bound to all interfaces with default route "
			// 					"interface on {} ( {} )\n",
			// 				pEndPointName, szIp, address.c_str()));
		}
		else
		{
			// 			ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
			// 				"Couldn't determine ip addr of default interface\n", pEndPointName));

			pEndPoint->close();
			return false;
		}
	}
	
	pEndPoint->setnonblocking(true); // ·Ç×èÈûI/O
	pEndPoint->setnodelay(true);
	pEndPoint->addr(address);
	
	if(rbuffer > 0)
	{
		if (!pEndPoint->setBufferSize(SO_RCVBUF, rbuffer))
		{
			// 			WARNING_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
			// 				"Operating with a receive buffer of only {} bytes (instead of {})\n",
			// 				pEndPointName, pEndPoint->getBufferSize(SO_RCVBUF), rbuffer));
		}
	}
	if(wbuffer > 0)
	{
		if (!pEndPoint->setBufferSize(SO_SNDBUF, wbuffer))
		{
			// 			WARNING_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
			// 				"Operating with a send buffer of only {} bytes (instead of {})\n",
			// 				pEndPointName, pEndPoint->getBufferSize(SO_SNDBUF), wbuffer));
		}
	}

	int backlog = gListenQ;
	if(backlog < 5)
		backlog = 5;

	if(pEndPoint->listen(backlog) == -1)
	{
		// 		ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
		// 			"listen to {} ({})\n",
		// 			pEndPointName, address.c_str(), __strerror()));

		pEndPoint->close();
		return false;
	}
	
	// 	INFO_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): address {}, SOMAXCONN={}.\n", 
	// 		pEndPointName, address.c_str(), backlog));

	return true;
}

void NetworkManager::processChannels(MessageHandlers* pMsgHandlers)
{
	ChannelMap::iterator iter = mChannels.begin();
	for(; iter != mChannels.end(); )
	{
		Channel* pChannel = iter->second;

		if(pChannel->isDestroyed())
		{
			++iter;
		}
		else if(pChannel->isCondemn())
		{
			++iter;

			deregisterChannel(pChannel);
			pChannel->destroy();
			Channel::ObjPool().reclaimObject(pChannel);
		}
		else
		{
			pChannel->processPackets(pMsgHandlers);
			++iter;
		}
	}
}

bool NetworkManager::registerChannel(Channel* pChannel)
{
	const Address &addr = pChannel->addr();

	Assert(addr.ip != 0);
	Assert(&pChannel->getNetworkManager() == this);

	ChannelMap::iterator iter = mChannels.find(addr);
	Channel *pExisting = iter != mChannels.end() ? iter->second : NULL;

	if(pExisting)
	{
		// 		CRITICAL_MSG(fmt::format("NetworkInterface::registerChannel: channel {} is exist.\n",
		// 			pChannel->c_str()));
		return false;
	}

	mChannels[addr] = pChannel;

	if(pChannel->isExternal())
		mNumExtChannels++;

	return true;
}

bool NetworkManager::deregisterChannel(Channel* pChannel)
{
	const Address &addr = pChannel->addr();
	Assert(pChannel->pEndPoint() != NULL);

	if(pChannel->isExternal())
		mNumExtChannels--;

	if (!mChannels.erase(addr))
	{
		// 		CRITICAL_MSG(fmt::format("NetworkInterface::deregisterChannel: "
		// 			"Channel not found {}!\n",
		// 			pChannel->c_str()));

		return false;
	}

	if(mpChannelDeregisterHandler)
	{
		mpChannelDeregisterHandler->onChannelDeregister(pChannel);
	}	

	return true;
}

bool NetworkManager::deregisterAllChannels()
{
	ChannelMap::iterator iter = mChannels.begin();
	while (iter != mChannels.end())
	{
		ChannelMap::iterator oldIter = iter++;
		Channel *pChannel = oldIter->second;
		pChannel->destroy();
		Channel::ObjPool().reclaimObject(pChannel);
	}

	mChannels.clear();
	mNumExtChannels = 0;

	return true;
}

Channel* NetworkManager::findChannel(const Address &addr)
{
	if (addr.ip == 0)
		return NULL;

	ChannelMap::iterator iter = mChannels.find(addr);
	Channel *pChannel = (iter != mChannels.end()) ? iter->second : NULL;
	return pChannel;
}

Channel* NetworkManager::findChannel(int fd)
{
	ChannelMap::iterator iter = mChannels.begin();
	for(; iter != mChannels.end(); ++iter)
	{
		if(iter->second->pEndPoint() && *iter->second->pEndPoint() == fd)
			return iter->second;
	}

	return NULL;
}

void NetworkManager::onChannelTimeOut(Channel * pChannel)
{
	if (mpChannelTimeOutHandler)
	{
		mpChannelTimeOutHandler->onChannelTimeOut(pChannel);
	}
	else
	{
		// 		ERROR_MSG(fmt::format("NetworkInterface::onChannelTimeOut: "
		// 			"Channel {} timed out but no handler is registered.\n",
		// 			pChannel->c_str()));
	}
}

void NetworkManager::delayedSend(Channel &channel)
{
	mpDelayedChannels->add(channel);
}

void NetworkManager::sendIfDelayed(Channel &channel)
{
	mpDelayedChannels->sendIfDelayed(channel);
}

void NetworkManager::onTimeout(TimerHandle handle, void *arg)
{
	// 	INFO_MSG(fmt::format("NetworkInterface::onTimeout: External({}), INTERNAL({}).\n", 
	// 		extaddr().c_str(), intaddr().c_str()));
}

void NetworkManager::closeSocket()
{
	if (mExtEndpoint.good())
	{
		this->dispatcher().deregisterReadFileDescriptor(mExtEndpoint);
		mExtEndpoint.close();
	}

	if (mIntEndpoint.good())
	{
		this->dispatcher().deregisterReadFileDescriptor(mIntEndpoint);
		mIntEndpoint.close();
	}
}
