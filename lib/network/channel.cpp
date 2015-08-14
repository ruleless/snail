#include "Channel.h"
#include "network/websocket_protocol.h"
#include "network/websocket_packet_filter.h"
#include "network/websocket_packet_reader.h"
#include "network/bundle.h"
#include "network/packet_reader.h"
#include "network/NetworkManager.h"
#include "network/TCPPacketReceiver.h"
#include "network/tcp_packet_sender.h"
#include "network/UDPPacketReceiver.h"
#include "network/TCPPacket.h"
#include "network/UDPPacket.h"
#include "network/message_handler.h"
#include "network/network_stats.h"


static ObjectPool<Channel> s_ObjPool("Channel");
ObjectPool<Channel>& Channel::ObjPool()
{
	return s_ObjPool;
}

void Channel::destroyObjPool()
{
	DEBUG_MSG(fmt::format("Channel::destroyObjPool(): size {}.\n", 
		s_ObjPool.size()));

	s_ObjPool.destroy();
}

size_t Channel::getPoolObjectBytes()
{
	size_t bytes = sizeof(mpNetworkManager) + sizeof(mTraits) + 
		sizeof(mID) + sizeof(mInactivityTimerHandle) + sizeof(mInactivityExceptionPeriod) + 
		sizeof(mLastReceivedTime) + (mBufferedReceives.size() * sizeof(Packet*)) + sizeof(mpPacketReader)
		+ sizeof(mFlags) + sizeof(mNumPacketsSent) + sizeof(mNumPacketsReceived) + sizeof(mNumBytesSent) + sizeof(mNumBytesReceived)
		+ sizeof(mLastTickBytesReceived) + sizeof(mLastTickBytesSent) + sizeof(mpFilter) + sizeof(mpEndPoint) + sizeof(mpPacketReceiver) + sizeof(mpPacketSender)
		+ sizeof(proxyID_) + mStrExtra.size() + sizeof(mChannelType)
		+ sizeof(mComponentID) + sizeof(mpMsgHandlers);

	return bytes;
}

void Channel::onReclaimObject()
{
	this->clearState();
}


Channel::Channel(NetworkManager & networkInterface,
				 const EndPoint * pEndPoint,
				 ETraits traits, ProtocolType pt,
				 PacketFilterPtr pFilter, ChannelID id)
				 :mpNetworkManager(NULL)
				 ,mTraits(traits)
				 ,mProtocolType(pt)
				 ,mID(id)
				 ,mInactivityTimerHandle()
				 ,mInactivityExceptionPeriod(0)
				 ,mLastReceivedTime(0)
				 ,mBundles()
				 ,mpPacketReader(0)
				 ,mNumPacketsSent(0)
				 ,mNumPacketsReceived(0)
				 ,mNumBytesSent(0)
				 ,mNumBytesReceived(0)
				 ,mLastTickBytesReceived(0)
				 ,mLastTickBytesSent(0)
				 ,mpFilter(pFilter)
				 ,mpEndPoint(NULL)
				 ,mpPacketReceiver(NULL)
				 ,mpPacketSender(NULL)
				 ,proxyID_(0)
				 ,mStrExtra()
				 ,mChannelType(Chanel_Normal)
				 ,mComponentID(UNKNOWN_COMPONENT_TYPE)
				 ,mpMsgHandlers(NULL)
				 ,mFlags(0)
{
	this->clearBundle();
	initialize(networkInterface, pEndPoint, traits, pt, pFilter, id);
}

Channel::Channel()
:mpNetworkManager(NULL)
,mTraits(External)
,mProtocolType(Protocol_TCP)
,mID(0)
,mInactivityTimerHandle()
,mInactivityExceptionPeriod(0)
,mLastReceivedTime(0)
,mBundles()
,mpPacketReader(0)
,mNumPacketsSent(0)
,mNumPacketsReceived(0)
,mNumBytesSent(0)
,mNumBytesReceived(0)
,mLastTickBytesReceived(0)
,mLastTickBytesSent(0)
,mpFilter(NULL)
,mpEndPoint(NULL)
,mpPacketReceiver(NULL)
,mpPacketSender(NULL)
,proxyID_(0)
,mStrExtra()
,mChannelType(Chanel_Normal)
,mComponentID(UNKNOWN_COMPONENT_TYPE)
,mpMsgHandlers(NULL)
,mFlags(0)
{
	this->clearBundle();
}

Channel::~Channel()
{
	finalise();
}

const char* Channel::c_str() const
{
	static char dodgyString[MAX_BUF] = {"None"};
	char tdodgyString[MAX_BUF] = {0};

	if(mpEndPoint && !mpEndPoint->addr().isNone())
		mpEndPoint->addr().writeToString(tdodgyString, MAX_BUF);

	kbe_snprintf(dodgyString, MAX_BUF, "%s/%d/%d/%d", tdodgyString, mID, this->isCondemn(), this->isDestroyed());

	return dodgyString;
}

bool Channel::initialize(NetworkManager &networkMgr,
						 const EndPoint *pEndPoint, 
						 ETraits traits, 
						 ProtocolType protoType, 
						 PacketFilterPtr pFilter, 
						 ChannelID id)
{
	mID = id;
	mProtocolType = protoType;
	mTraits = traits;
	mpFilter = pFilter;
	mpNetworkManager = &networkMgr;
	this->pEndPoint(pEndPoint);

	Assert(mpNetworkManager != NULL);
	Assert(mpEndPoint != NULL);

	if(mProtocolType == Protocol_TCP)
	{
		if(mpPacketReceiver)
		{
			if(mpPacketReceiver->type() == PacketReceiver::ReceiverType_UDP)
			{
				SafeDelete(mpPacketReceiver);
				mpPacketReceiver = new TCPPacketReceiver(*mpEndPoint, *mpNetworkManager);
			}
		}
		else
		{
			mpPacketReceiver = new TCPPacketReceiver(*mpEndPoint, *mpNetworkManager);
		}

		mpNetworkManager->dispatcher().registerReadFileDescriptor(*mpEndPoint, mpPacketReceiver);

		// 需要发送数据时再注册
		// mpPacketSender = new TCPPacketSender(*pEndPoint_, *pNetworkInterface_);
		// mpNetworkManager->dispatcher().registerWriteFileDescriptor(*pEndPoint_, pPacketSender_);
	}
	else
	{
		if(mpPacketReceiver)
		{
			if(mpPacketReceiver->type() == PacketReceiver::ReceiverType_TCP)
			{
				SafeDelete(mpPacketReceiver);
				mpPacketReceiver = new UDPPacketReceiver(*mpEndPoint, *mpNetworkManager);
			}
		}
		else
		{
			mpPacketReceiver = new UDPPacketReceiver(*mpEndPoint, *mpNetworkManager);
		}
	}

	mpPacketReceiver->setEndPoint(mpEndPoint);
	if(mpPacketSender)
		mpPacketSender->pEndPoint(mpEndPoint);

	startInactivityDetection((mTraits == Internal) ? gChannelInternalTimeout : gChannelExternalTimeout,
		(mTraits == Internal) ? gChannelInternalTimeout / 2.f : gChannelExternalTimeout / 2.f);

	return true;
}

bool Channel::finalise()
{
	this->clearState();
	
	SafeDelete(mpPacketReceiver);
	SafeDelete(mpPacketReader);
	SafeDelete(mpPacketSender);

	EndPoint::ObjPool().reclaimObject(mpEndPoint);
	mpEndPoint = NULL;

	return true;
}

void Channel::destroy()
{
	if(isDestroyed())
	{
		CRITICAL_MSG("is channel has Destroyed!\n");
		return;
	}

	clearState();
	mFlags |= Flag_Destroyed;
}

void Channel::clearState(bool warnOnDiscard)
{
	if (mBufferedReceives.size() > 0)
	{
		BufferedReceives::iterator iter = mBufferedReceives.begin();
		int hasDiscard = 0;

		for(; iter != mBufferedReceives.end(); ++iter)
		{
			Packet* pPacket = (*iter);
			if(pPacket->length() > 0)
				hasDiscard++;

			reclaimPacket(pPacket->isTCPPacket(), pPacket);
		}

		if (hasDiscard > 0 && warnOnDiscard)
		{
			WARNING_MSG(fmt::format("Channel::clearState( {} ): "
				"Discarding {} buffered packet(s)\n",
				this->c_str(), hasDiscard));
		}

		mBufferedReceives.clear();
	}

	clearBundle();

	mLastReceivedTime = timestamp();

	mNumPacketsSent = 0;
	mNumPacketsReceived = 0;
	mNumBytesSent = 0;
	mNumBytesReceived = 0;
	mLastTickBytesReceived = 0;
	proxyID_ = 0;
	mStrExtra = "";
	mChannelType = Chanel_Normal;

	if(mpEndPoint && mProtocolType == Protocol_TCP && !this->isDestroyed())
	{
		this->stopSend();

		if(mpNetworkManager)
		{
			if(!this->isDestroyed())
				mpNetworkManager->dispatcher().deregisterReadFileDescriptor(*mpEndPoint);
		}
	}

	mFlags = 0;
	mpFilter = NULL;

	stopInactivityDetection();

	if(mpEndPoint)
	{
		mpEndPoint->close();
		this->pEndPoint(NULL);
	}
}

void Channel::startInactivityDetection( float period, float checkPeriod )
{
	stopInactivityDetection();

	// 如果周期为负数则不检查
	if(period > 0.001f)
	{
		checkPeriod = std::max(1.f, checkPeriod);
		mInactivityExceptionPeriod = uint64(period * stampsPerSecond() ) - uint64( 0.05f * stampsPerSecond());
		mLastReceivedTime = timestamp();

		mInactivityTimerHandle = mpNetworkManager->dispatcher().addTimer(int( checkPeriod * 1000000 ), this, (void *)Timeout_InactivityCheck);
	}
}

void Channel::stopInactivityDetection()
{
	mInactivityTimerHandle.cancel();
}

void Channel::pEndPoint(const EndPoint* pEndPoint)
{
	if (mpEndPoint != pEndPoint)
	{
		EndPoint::ObjPool().reclaimObject(mpEndPoint);
		mpEndPoint = const_cast<EndPoint*>(pEndPoint);
	}
	
	mLastReceivedTime = timestamp();
}

int32 Channel::bundlesLength()
{
	int32 len = 0;
	Bundles::iterator iter = mBundles.begin();
	for(; iter != mBundles.end(); ++iter)
	{
		len += (*iter)->packetsLength();
	}

	return len;
}

void Channel::clearBundle()
{
	Bundles::iterator iter = mBundles.begin();
	for(; iter != mBundles.end(); ++iter)
	{
		Bundle::ObjPool().reclaimObject((*iter));
	}

	mBundles.clear();
}

void Channel::onTimeout(TimerHandle, void *arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case Timeout_InactivityCheck:
		{
			if (timestamp() - mLastReceivedTime >= mInactivityExceptionPeriod)
			{
				this->getNetworkManager().onChannelTimeOut(this);
			}
			break;
		}
		default:
			break;
	}
}

void Channel::send(Bundle *pBundle)
{
	if (isDestroyed())
	{
		ERROR_MSG(fmt::format("Channel::send({}): channel has destroyed.\n", this->c_str()));
		
		this->clearBundle();

		if(pBundle)
			Bundle::ObjPool().reclaimObject(pBundle);

		return;
	}

	if(isCondemn())
	{
		this->clearBundle();

		if(pBundle)
			Bundle::ObjPool().reclaimObject(pBundle);

		return;
	}

	if(pBundle)
	{
		pBundle->finiMessage(true);
		mBundles.push_back(pBundle);
	}
	
	uint32 bundleSize = (uint32)mBundles.size();
	if(bundleSize == 0)
		return;

	if(!sending())
	{
		if(mpPacketSender == NULL)
			mpPacketSender = new TCPPacketSender(*mpEndPoint, *mpNetworkManager);

		mpPacketSender->processSend(this);

		if(mBundles.size() > 0 && !isCondemn() && !isDestroyed())
		{
			mFlags |= Flag_Sending;
			mpNetworkManager->dispatcher().registerWriteFileDescriptor(*mpEndPoint, mpPacketSender);
		}
	}

	if(gSendWindowMessagesOverflowCritical > 0 && bundleSize > gSendWindowMessagesOverflowCritical)
	{
		if(this->isExternal())
		{
			WARNING_MSG(fmt::format("Channel::send[{:p}]: external channel({}), send window has overflowed({} > {}).\n", 
				(void*)this, this->c_str(), bundleSize, gSendWindowMessagesOverflowCritical));

			if(gExtSendWindowMessagesOverflow > 0 && bundleSize > gExtSendWindowMessagesOverflow)
			{
				ERROR_MSG(fmt::format("Channel::send[{:p}]: external channel({}), send window has overflowed({} > {}), Try adjusting the kbengine_defs.xml->windowOverflow->send.\n", 
					(void*)this, this->c_str(), bundleSize, gExtSendWindowMessagesOverflow));

				this->condemn();
			}
		}
		else
		{
			if(gIntSendWindowMessagesOverflow > 0 && bundleSize > gIntSendWindowMessagesOverflow)
			{
				ERROR_MSG(fmt::format("Channel::send[{:p}]: internal channel({}), send window has overflowed({} > {}).\n", 
					(void*)this, this->c_str(), bundleSize, gIntSendWindowMessagesOverflow));

				this->condemn();
			}
			else
			{
				WARNING_MSG(fmt::format("Channel::send[{:p}]: internal channel({}), send window has overflowed({} > {}).\n", 
					(void*)this, this->c_str(), bundleSize, gSendWindowMessagesOverflowCritical));
			}
		}
	}
}

void Channel::stopSend()
{
	if(!sending())
		return;

	mFlags &= ~Flag_Sending;

	mpNetworkManager->dispatcher().deregisterWriteFileDescriptor(*mpEndPoint);
}

void Channel::delayedSend()
{
	this->getNetworkManager().delayedSend(*this);
}

void Channel::onSendCompleted()
{
	Assert(mBundles.size() == 0 && sending());
	stopSend();
}

void Channel::onPacketSent(int bytes, bool sentCompleted)
{
	if(sentCompleted)
	{
		++mNumPacketsSent;
		++g_numPacketsSent;
	}

	mNumBytesSent += bytes;
	g_numBytesSent += bytes;
	mLastTickBytesSent += bytes;

	if(this->isExternal())
	{
		if(g_extSendWindowBytesOverflow > 0 && mLastTickBytesSent >= g_extSendWindowBytesOverflow)
		{
			ERROR_MSG(fmt::format("Channel::onPacketSent[{:p}]: external channel({}), bufferedBytes has overflowed({} > {}), Try adjusting the kbengine_defs.xml->windowOverflow->receive.\n", 
				(void*)this, this->c_str(), mLastTickBytesSent, g_extSendWindowBytesOverflow));

			this->condemn();
		}
	}
	else
	{
		if(g_intSendWindowBytesOverflow > 0 && mLastTickBytesSent >= g_intSendWindowBytesOverflow)
		{
			WARNING_MSG(fmt::format("Channel::onPacketSent[{:p}]: internal channel({}), bufferedBytes has overflowed({} > {}).\n", 
				(void*)this, this->c_str(), mLastTickBytesSent, g_intSendWindowBytesOverflow));
		}
	}
}

void Channel::onPacketReceived(int bytes)
{
	mLastReceivedTime = timestamp();
	++mNumPacketsReceived;
	++g_numPacketsReceived;

	mNumBytesReceived += bytes;
	mLastTickBytesReceived += bytes;
	g_numBytesReceived += bytes;

	if(this->isExternal())
	{
		if(g_extReceiveWindowBytesOverflow > 0 && mLastTickBytesReceived >= g_extReceiveWindowBytesOverflow)
		{
			ERROR_MSG(fmt::format("Channel::onPacketReceived[{:p}]: external channel({}), bufferedBytes has overflowed({} > {}), Try adjusting the kbengine_defs.xml->windowOverflow->receive.\n", 
				(void*)this, this->c_str(), mLastTickBytesReceived, g_extReceiveWindowBytesOverflow));

			this->condemn();
		}
	}
	else
	{
		if(g_intReceiveWindowBytesOverflow > 0 && mLastTickBytesReceived >= g_intReceiveWindowBytesOverflow)
		{
			WARNING_MSG(fmt::format("Channel::onPacketReceived[{:p}]: internal channel({}), bufferedBytes has overflowed({} > {}).\n", 
				(void*)this, this->c_str(), mLastTickBytesReceived, g_intReceiveWindowBytesOverflow));
		}
	}
}

void Channel::addReceiveWindow(Packet* pPacket)
{
	mBufferedReceives.push_back(pPacket);
	uint32 size = (uint32)mBufferedReceives.size();

	if(g_receiveWindowMessagesOverflowCritical > 0 && size > g_receiveWindowMessagesOverflowCritical)
	{
		if(this->isExternal())
		{
			if(g_extReceiveWindowMessagesOverflow > 0 && size > g_extReceiveWindowMessagesOverflow)
			{
				ERROR_MSG(fmt::format("Channel::addReceiveWindow[{:p}]: external channel({}), receive window has overflowed({} > {}), Try adjusting the kbengine_defs.xml->windowOverflow->receive->messages->external.\n", 
					(void*)this, this->c_str(), size, g_extReceiveWindowMessagesOverflow));

				this->condemn();
			}
			else
			{
				WARNING_MSG(fmt::format("Channel::addReceiveWindow[{:p}]: external channel({}), receive window has overflowed({} > {}).\n", 
					(void*)this, this->c_str(), size, g_receiveWindowMessagesOverflowCritical));
			}
		}
		else
		{
			if(g_intReceiveWindowMessagesOverflow > 0 && size > g_intReceiveWindowMessagesOverflow)
			{
				WARNING_MSG(fmt::format("Channel::addReceiveWindow[{:p}]: internal channel({}), receive window has overflowed({} > {}).\n", 
					(void*)this, this->c_str(), size, g_intReceiveWindowMessagesOverflow));
			}
		}
	}
}

void Channel::processPackets(MessageHandlers* pMsgHandlers)
{
	mLastTickBytesReceived = 0;
	mLastTickBytesSent = 0;

	if(mpMsgHandlers != NULL)
	{
		pMsgHandlers = mpMsgHandlers;
	}

	if (this->isDestroyed())
	{
		ERROR_MSG(fmt::format("Channel::processPackets({}): channel[{:p}] is destroyed.\n", 
			this->c_str(), (void*)this));

		return;
	}

	if(this->isCondemn())
	{
		ERROR_MSG(fmt::format("Channel::processPackets({}): channel[{:p}] is condemn.\n", 
			this->c_str(), (void*)this));

		return;
	}

	if(!hasHandshake())
	{
		handshake();
	}

	try
	{
		BufferedReceives::iterator packetIter = mBufferedReceives.begin();
		for(; packetIter != mBufferedReceives.end(); ++packetIter)
		{
			Packet* pPacket = (*packetIter);
			mpPacketReader->processMessages(pMsgHandlers, pPacket);
			reclaimPacket(pPacket->isTCPPacket(), pPacket);
		}
	}
	catch(MemoryStreamException &)
	{
		MessageHandler* pMsgHandler = pMsgHandlers->find(mpPacketReader->currMsgID());
		WARNING_MSG(fmt::format("Channel::processPackets({}): packet invalid. currMsg=(name={}, id={}, len={}), currMsgLen={}\n",
			this->c_str()
			, (pMsgHandler == NULL ? "unknown" : pMsgHandler->name) 
			, mpPacketReader->currMsgID() 
			, (pMsgHandler == NULL ? -1 : pMsgHandler->msgLen) 
			, mpPacketReader->currMsgLen()));

		mpPacketReader->currMsgID(0);
		mpPacketReader->currMsgLen(0);
		condemn();
	}

	mBufferedReceives.clear();
}

void Channel::condemn()
{ 
	if(isCondemn())
		return;

	mFlags |= Flag_Condemn; 
}

void Channel::handshake()
{
	if(hasHandshake())
		return;

	if(mBufferedReceives.size() > 0)
	{
		BufferedReceives::iterator packetIter = mBufferedReceives.begin();
		Packet* pPacket = (*packetIter);
		
		mFlags |= Flag_HandShake;

		// 此处判定是否为websocket或者其他协议的握手
		if(WebSocketProtocol::isWebSocketProtocol(pPacket))
		{
			mChannelType = Chanel_Web;
			if(WebSocketProtocol::handshake(this, pPacket))
			{
				if(pPacket->length() == 0)
				{
					mBufferedReceives.erase(packetIter);
				}

				if(!mpPacketReader || mpPacketReader->type() != PacketReader::PACKET_READER_TYPE_WEBSOCKET)
				{
					SafeDelete(mpPacketReader);
					mpPacketReader = new WebSocketPacketReader(this);
				}

				mpFilter = new WebSocketPacketFilter(this);
				DEBUG_MSG(fmt::format("Channel::handshake: websocket({}) successfully!\n", this->c_str()));
				return;
			}
			else
			{
				DEBUG_MSG(fmt::format("Channel::handshake: websocket({}) error!\n", this->c_str()));
			}
		}

		if(!mpPacketReader || mpPacketReader->type() != PacketReader::PACKET_READER_TYPE_SOCKET)
		{
			SafeDelete(mpPacketReader);
			mpPacketReader = new PacketReader(this);
		}

		mpPacketReader->reset();
	}
}

bool Channel::waitSend()
{
	return pEndPoint()->waitSend();
}