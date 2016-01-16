#include "TCPPacketSender.h"
#include "Address.h"
#include "Bundle.h"
#include "Channel.h"
#include "EndPoint.h"
#include "EventDispatcher.h"
#include "NetworkManager.h"
#include "TCPPacket.h"

static ObjectPool<TCPPacketSender> s_ObjPool("TCPPacketSender");
ObjectPool<TCPPacketSender>& TCPPacketSender::ObjPool()
{
	return s_ObjPool;
}

void TCPPacketSender::destroyObjPool()
{
// 	DEBUG_MSG(fmt::format("TCPPacketSender::destroyObjPool(): size {}.\n", 
// 		s_ObjPool.size()));

	s_ObjPool.destroy();
}

TCPPacketSender::TCPPacketSender(EndPoint &endpoint, NetworkManager & networkInterface) 
:PacketSender(endpoint, networkInterface)
{
}

TCPPacketSender::~TCPPacketSender()
{
}

void TCPPacketSender::onGetError(Channel* pChannel)
{
	pChannel->condemn();
}

bool TCPPacketSender::processSend(Channel* pChannel)
{
	bool noticed = pChannel == NULL;

	if(noticed)
		pChannel = getChannel();

	Assert(pChannel != NULL);
	
	if(pChannel->isCondemn())
	{
		return false;
	}
	
	Channel::Bundles& bundles = pChannel->bundles();
	EReason reason = Reason_Success;

	Channel::Bundles::iterator iter = bundles.begin();
	for(; iter != bundles.end(); ++iter)
	{
		Bundle::Packets& pakcets = (*iter)->packets();
		Bundle::Packets::iterator iter1 = pakcets.begin();
		for (; iter1 != pakcets.end(); ++iter1)
		{
			reason = processPacket(pChannel, (*iter1));
			if(reason != Reason_Success)
				break; 
			else
				reclaimPacket((*iter)->isTCPPacket(), (*iter1));
		}

		if(reason == Reason_Success)
		{
			pakcets.clear();
			Bundle::ObjPool().reclaimObject((*iter));
		}
		else
		{
			pakcets.erase(pakcets.begin(), iter1);
			bundles.erase(bundles.begin(), iter);

			if (reason == Reason_ResourceUnavailable)
			{
				/* 此处输出可能会造成debugHelper处死锁
					WARNING_MSG(fmt::format("TCPPacketSender::processSend: "
						"Transmit queue full, waiting for space(kbengine.xml->channelCommon->writeBufferSize->{})...\n",
						(pChannel->isInternal() ? "internal" : "external")));
				*/
			}
			else
			{
				onGetError(pChannel);
			}

			return false;
		}
	}

	bundles.clear();

	if(noticed)
		pChannel->onSendCompleted();

	return true;
}

EReason TCPPacketSender::processFilterPacket(Channel *pChannel, Packet *pPacket)
{
	if(pChannel->isCondemn())
	{
		return Reason_ChannelCondemn;
	}

	EndPoint* pEndpoint = pChannel->pEndPoint();
	int len = pEndpoint->send(pPacket->data() + pPacket->sentSize, pPacket->length() - pPacket->sentSize);

	if(len > 0)
	{
		pPacket->sentSize += len;
	}

	bool sentCompleted = pPacket->sentSize == pPacket->length();
	pChannel->onPacketSent(len, sentCompleted);

	if (sentCompleted)
		return Reason_Success;

	return checkSocketErrors(pEndpoint);
}
