/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "network_interface.h"
#ifndef _INLINE
#include "network_interface.inl"
#endif

#include "network/address.h"
#include "network/event_dispatcher.h"
#include "network/packet_receiver.h"
#include "network/listener_receiver.h"
#include "network/channel.h"
#include "network/packet.h"
#include "network/delayed_channels.h"
#include "network/interfaces.h"
#include "network/message_handler.h"

namespace KBEngine { 
namespace Network
{

//-------------------------------------------------------------------------------------
NetworkManager::NetworkManager(Network::EventDispatcher * pDispatcher,
		int32 extlisteningPort_min, int32 extlisteningPort_max, const char * extlisteningInterface,
		uint32 extrbuffer, uint32 extwbuffer,
		int32 intlisteningPort, const char * intlisteningInterface,
		uint32 intrbuffer, uint32 intwbuffer):
	extEndpoint_(),
	intEndpoint_(),
	channelMap_(),
	pDispatcher_(pDispatcher),
	pExtensionData_(NULL),
	pExtListenerReceiver_(NULL),
	pIntListenerReceiver_(NULL),
	pDelayedChannels_(new DelayedChannels()),
	pChannelTimeOutHandler_(NULL),
	pChannelDeregisterHandler_(NULL),
	isExternal_(extlisteningPort_min != -1),
	numExtChannels_(0)
{
	if(isExternal())
	{
		pExtListenerReceiver_ = new ListenerReceiver(extEndpoint_, Channel::EXTERNAL, *this);
		this->recreateListeningSocket("EXTERNAL", htons(extlisteningPort_min), htons(extlisteningPort_max), 
			extlisteningInterface, &extEndpoint_, pExtListenerReceiver_, extrbuffer, extwbuffer);

		// 如果配置了对外端口范围， 如果范围过小这里extEndpoint_可能没有端口可用了
		if(extlisteningPort_min != -1)
		{
			Assert(extEndpoint_.good() && "Channel::EXTERNAL: no available port, "
				"please check for kbengine_defs.xml!\n");
		}
	}

	if(intlisteningPort != -1)
	{
		pIntListenerReceiver_ = new ListenerReceiver(intEndpoint_, Channel::INTERNAL, *this);
		this->recreateListeningSocket("INTERNAL", intlisteningPort, intlisteningPort, 
			intlisteningInterface, &intEndpoint_, pIntListenerReceiver_, intrbuffer, intwbuffer);
	}

	Assert(good() && "NetworkInterface::NetworkInterface: no available port, "
		"please check for kbengine_defs.xml!\n");

	pDelayedChannels_->init(this->dispatcher(), this);
}

//-------------------------------------------------------------------------------------
NetworkManager::~NetworkManager()
{
	ChannelMap::iterator iter = channelMap_.begin();
	while (iter != channelMap_.end())
	{
		ChannelMap::iterator oldIter = iter++;
		Channel * pChannel = oldIter->second;
		pChannel->destroy();
		delete pChannel;
	}

	channelMap_.clear();

	this->closeSocket();

	if (pDispatcher_ != NULL)
	{
		pDelayedChannels_->fini(this->dispatcher());
		pDispatcher_ = NULL;
	}

	SAFE_RELEASE(pDelayedChannels_);
	SAFE_RELEASE(pExtListenerReceiver_);
	SAFE_RELEASE(pIntListenerReceiver_);
}

//-------------------------------------------------------------------------------------
void NetworkManager::closeSocket()
{
	if (extEndpoint_.good())
	{
		this->dispatcher().deregisterReadFileDescriptor(extEndpoint_);
		extEndpoint_.close();
	}

	if (intEndpoint_.good())
	{
		this->dispatcher().deregisterReadFileDescriptor(intEndpoint_);
		intEndpoint_.close();
	}
}

//-------------------------------------------------------------------------------------
bool NetworkManager::recreateListeningSocket(const char* pEndPointName, uint16 listeningPort_min, uint16 listeningPort_max, 
										const char * listeningInterface, EndPoint* pEP, ListenerReceiver* pLR, uint32 rbuffer, 
										uint32 wbuffer)
{
	Assert(listeningInterface && pEP && pLR);

	if (pEP->good())
	{
		this->dispatcher().deregisterReadFileDescriptor(*pEP);
		pEP->close();
	}

	Address address;
	address.ip = 0;
	address.port = 0;

	pEP->socket(SOCK_STREAM);
	if (!pEP->good())
	{
		ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): couldn't create a socket\n",
			pEndPointName));

		return false;
	}
	
	/*
		pEP->setreuseaddr(true);
	*/
	
	this->dispatcher().registerReadFileDescriptor(*pEP, pLR);
	
	u_int32_t ifIPAddr = INADDR_ANY;

	bool listeningInterfaceEmpty =
		(listeningInterface == NULL || listeningInterface[0] == 0);

	// 查找指定接口名 NIP、MAC、IP是否可用
	if(pEP->findIndicatedInterface(listeningInterface, ifIPAddr) == 0)
	{
		char szIp[MAX_IP] = {0};
		Address::ip2string(ifIPAddr, szIp);

		INFO_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Creating on interface '{}' (= {})\n",
			pEndPointName, listeningInterface, szIp));
	}

	// 如果不为空又找不到那么警告用户错误的设置，同时我们采用默认的方式(绑定到INADDR_ANY)
	else if (!listeningInterfaceEmpty)
	{
		WARNING_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Couldn't parse interface spec '{}' so using all interfaces\n",
			pEndPointName, listeningInterface));
	}
	
	// 尝试绑定到端口，如果被占用向后递增
	bool foundport = false;
	uint32 listeningPort = listeningPort_min;
	if(listeningPort_min != listeningPort_max)
	{
		for(int lpIdx=ntohs(listeningPort_min); lpIdx<ntohs(listeningPort_max); ++lpIdx)
		{
			listeningPort = htons(lpIdx);
			if (pEP->bind(listeningPort, ifIPAddr) != 0)
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
		if (pEP->bind(listeningPort, ifIPAddr) == 0)
		{
			foundport = true;
		}
	}

	// 如果无法绑定到合适的端口那么报错返回，进程将退出
	if(!foundport)
	{
		ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Couldn't bind the socket to {}:{} ({})\n",
			pEndPointName, inet_ntoa((struct in_addr&)ifIPAddr), ntohs(listeningPort), kbe_strerror()));
		
		pEP->close();
		return false;
	}

	// 获得当前绑定的地址，如果是INADDR_ANY这里获得的IP是0
	pEP->getlocaladdress( (u_int16_t*)&address.port,
		(u_int32_t*)&address.ip );

	if (0 == address.ip)
	{
		u_int32_t addr;
		if(0 == pEP->getDefaultInterfaceAddress(addr))
		{
			address.ip = addr;

			char szIp[MAX_IP] = {0};
			Address::ip2string(address.ip, szIp);
			INFO_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
					"bound to all interfaces with default route "
					"interface on {} ( {} )\n",
				pEndPointName, szIp, address.c_str()));
		}
		else
		{
			ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Couldn't determine ip addr of default interface\n", pEndPointName));

			pEP->close();
			return false;
		}
	}
	
	pEP->setnonblocking(true);
	pEP->setnodelay(true);
	pEP->addr(address);
	
	if(rbuffer > 0)
	{
		if (!pEP->setBufferSize(SO_RCVBUF, rbuffer))
		{
			WARNING_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Operating with a receive buffer of only {} bytes (instead of {})\n",
				pEndPointName, pEP->getBufferSize(SO_RCVBUF), rbuffer));
		}
	}
	if(wbuffer > 0)
	{
		if (!pEP->setBufferSize(SO_SNDBUF, wbuffer))
		{
			WARNING_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
				"Operating with a send buffer of only {} bytes (instead of {})\n",
				pEndPointName, pEP->getBufferSize(SO_SNDBUF), wbuffer));
		}
	}

	int backlog = Network::g_SOMAXCONN;
	if(backlog < 5)
		backlog = 5;

	if(pEP->listen(backlog) == -1)
	{
		ERROR_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): "
			"listen to {} ({})\n",
			pEndPointName, address.c_str(), kbe_strerror()));

		pEP->close();
		return false;
	}
	
	INFO_MSG(fmt::format("NetworkInterface::recreateListeningSocket({}): address {}, SOMAXCONN={}.\n", 
		pEndPointName, address.c_str(), backlog));

	return true;
}

//-------------------------------------------------------------------------------------
void NetworkManager::delayedSend(Channel & channel)
{
	pDelayedChannels_->add(channel);
}

//-------------------------------------------------------------------------------------
void NetworkManager::sendIfDelayed(Channel & channel)
{
	pDelayedChannels_->sendIfDelayed(channel);
}

//-------------------------------------------------------------------------------------
void NetworkManager::handleTimeout(TimerHandle handle, void * arg)
{
	INFO_MSG(fmt::format("NetworkInterface::handleTimeout: EXTERNAL({}), INTERNAL({}).\n", 
		extaddr().c_str(), intaddr().c_str()));
}

//-------------------------------------------------------------------------------------
Channel * NetworkManager::findChannel(const Address & addr)
{
	if (addr.ip == 0)
		return NULL;

	ChannelMap::iterator iter = channelMap_.find(addr);
	Channel * pChannel = (iter != channelMap_.end()) ? iter->second : NULL;
	return pChannel;
}

//-------------------------------------------------------------------------------------
Channel * NetworkManager::findChannel(int fd)
{
	ChannelMap::iterator iter = channelMap_.begin();
	for(; iter != channelMap_.end(); ++iter)
	{
		if(iter->second->pEndPoint() && *iter->second->pEndPoint() == fd)
			return iter->second;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
bool NetworkManager::registerChannel(Channel* pChannel)
{
	const Address & addr = pChannel->addr();
	Assert(addr.ip != 0);
	Assert(&pChannel->networkInterface() == this);
	ChannelMap::iterator iter = channelMap_.find(addr);
	Channel * pExisting = iter != channelMap_.end() ? iter->second : NULL;

	if(pExisting)
	{
		CRITICAL_MSG(fmt::format("NetworkInterface::registerChannel: channel {} is exist.\n",
		pChannel->c_str()));
		return false;
	}

	channelMap_[addr] = pChannel;

	if(pChannel->isExternal())
		numExtChannels_++;

	//INFO_MSG(fmt::format("NetworkInterface::registerChannel: new channel: {}.\n", pChannel->c_str()));
	return true;
}

//-------------------------------------------------------------------------------------
bool NetworkManager::deregisterAllChannels()
{
	ChannelMap::iterator iter = channelMap_.begin();
	while (iter != channelMap_.end())
	{
		ChannelMap::iterator oldIter = iter++;
		Channel * pChannel = oldIter->second;
		pChannel->destroy();
		Network::Channel::ObjPool().reclaimObject(pChannel);
	}

	channelMap_.clear();
	numExtChannels_ = 0;

	return true;
}

//-------------------------------------------------------------------------------------
bool NetworkManager::deregisterChannel(Channel* pChannel)
{
	const Address & addr = pChannel->addr();
	Assert(pChannel->pEndPoint() != NULL);

	if(pChannel->isExternal())
		numExtChannels_--;

	//INFO_MSG(fmt::format("NetworkInterface::deregisterChannel: del channel: {}\n",
	//	pChannel->c_str()));

	if (!channelMap_.erase(addr))
	{
		CRITICAL_MSG(fmt::format("NetworkInterface::deregisterChannel: "
				"Channel not found {}!\n",
			pChannel->c_str()));

		return false;
	}

	if(pChannelDeregisterHandler_)
	{
		pChannelDeregisterHandler_->onChannelDeregister(pChannel);
	}	

	return true;
}

//-------------------------------------------------------------------------------------
void NetworkManager::onChannelTimeOut(Channel * pChannel)
{
	if (pChannelTimeOutHandler_)
	{
		pChannelTimeOutHandler_->onChannelTimeOut(pChannel);
	}
	else
	{
		ERROR_MSG(fmt::format("NetworkInterface::onChannelTimeOut: "
					"Channel {} timed out but no handler is registered.\n",
				pChannel->c_str()));
	}
}

//-------------------------------------------------------------------------------------
void NetworkManager::processChannels(KBEngine::Network::MessageHandlers* pMsgHandlers)
{
	ChannelMap::iterator iter = channelMap_.begin();
	for(; iter != channelMap_.end(); )
	{
		Network::Channel* pChannel = iter->second;

		if(pChannel->isDestroyed())
		{
			++iter;
		}
		else if(pChannel->isCondemn())
		{
			++iter;

			deregisterChannel(pChannel);
			pChannel->destroy();
			Network::Channel::ObjPool().reclaimObject(pChannel);
		}
		else
		{
			pChannel->processPackets(pMsgHandlers);
			++iter;
		}
	}
}
//-------------------------------------------------------------------------------------
}
}
