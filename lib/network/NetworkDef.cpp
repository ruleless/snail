#include "NetworkDef.h"
#include "network/Channel.h"
#include "network/Bundle.h"
#include "network/TCPPacket.h"
#include "network/UDPPacket.h"
#include "network/MessageHandler.h"
#include "network/TCPPacketReceiver.h"
#include "network/UDPPacketReceiver.h"
#include "network/Address.h"
#include "helper/watcher.h"

float gChannelInternalTimeout = 60.f;
float gChannelExternalTimeout = 60.f;

int8 g_channelExternalEncryptType = 0;

uint32 gListenQ = 5;

// network stats
uint64						g_numPacketsSent = 0;
uint64						g_numPacketsReceived = 0;
uint64						g_numBytesSent = 0;
uint64						g_numBytesReceived = 0;

uint32						g_receiveWindowMessagesOverflowCritical = 32;
uint32						g_intReceiveWindowMessagesOverflow = 65535;
uint32						g_extReceiveWindowMessagesOverflow = 256;
uint32						g_intReceiveWindowBytesOverflow = 0;
uint32						g_extReceiveWindowBytesOverflow = 65535;

uint32						gSendWindowMessagesOverflowCritical = 32;
uint32						gIntSendWindowMessagesOverflow = 65535;
uint32						gExtSendWindowMessagesOverflow = 256;
uint32						g_intSendWindowBytesOverflow = 0;
uint32						g_extSendWindowBytesOverflow = 65535;

// 通道发送超时重试
uint32						g_intReSendInterval = 10;
uint32						g_intReSendRetries = 0;
uint32						g_extReSendInterval = 10;
uint32						g_extReSendRetries = 3;

bool initializeWatcher()
{
	WATCH_OBJECT("network/numPacketsSent", g_numPacketsSent);
	WATCH_OBJECT("network/numPacketsReceived", g_numPacketsReceived);
	WATCH_OBJECT("network/numBytesSent", g_numBytesSent);
	WATCH_OBJECT("network/numBytesReceived", g_numBytesReceived);
	
	std::vector<MessageHandlers*>::iterator iter = MessageHandlers::messageHandlers().begin();
	for(; iter != MessageHandlers::messageHandlers().end(); ++iter)
	{
		if(!(*iter)->initializeWatcher())
			return false;
	}

	return true;
}

void destroyObjPool()
{
	Bundle::destroyObjPool();
	Channel::destroyObjPool();
	TCPPacket::destroyObjPool();
	UDPPacket::destroyObjPool();
	EndPoint::destroyObjPool();
	Address::destroyObjPool();
	TCPPacketReceiver::destroyObjPool();
	UDPPacketReceiver::destroyObjPool();
}

Packet* mallocPacket(bool isTCPPacket)
{
	if(isTCPPacket)
		return TCPPacket::ObjPool().createObject();
	else
		return UDPPacket::ObjPool().createObject();
}

void reclaimPacket(bool isTCPPacket, Packet *pPacket)
{
	if(isTCPPacket)
		TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>(pPacket));
	else
		UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket*>(pPacket));
}

void finalise(void)
{
#ifdef ENABLE_WATCHERS
	WatcherPaths::finalise();
#endif

	MessageHandlers::finalise();
	
	destroyObjPool();
}
