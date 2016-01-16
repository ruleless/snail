#include "NetworkDef.h"
#include "Channel.h"
#include "Bundle.h"
#include "TCPPacket.h"
#include "UDPPacket.h"
#include "MessageHandler.h"
#include "TCPPacketReceiver.h"
#include "UDPPacketReceiver.h"
#include "Address.h"

uint32 gListenQ = 5;

// 通道超时时间
float gChannelInternalTimeout = 60.f;
float gChannelExternalTimeout = 60.f;

// 通道发送超时重试
uint32 gIntReSendInterval = 10;
uint32 gIntReSendRetries = 0;
uint32 gExtReSendInterval = 10;
uint32 gExtReSendRetries = 3;

// 外部通道加密类别
int8 gChannelExternalEncryptType = 0;

// network stats
uint64 gNumPacketsSent = 0;
uint64 gNumPacketsReceived = 0;
uint64 gNumBytesSent = 0;
uint64 gNumBytesReceived = 0;

// 包接收窗口溢出参数
uint32 gReceiveWindowMessagesOverflowCritical = 32;
uint32 gIntReceiveWindowMessagesOverflow = 65535;
uint32 gExtReceiveWindowMessagesOverflow = 256;
uint32 gIntReceiveWindowBytesOverflow = 0;
uint32 gExtReceiveWindowBytesOverflow = 65535;

// 包发送窗口溢出参数
uint32 gSendWindowMessagesOverflowCritical = 32;
uint32 gIntSendWindowMessagesOverflow = 65535;
uint32 gExtSendWindowMessagesOverflow = 256;
uint32 gIntSendWindowBytesOverflow = 0;
uint32 gExtSendWindowBytesOverflow = 65535;

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

void finalise(void)
{
	destroyObjPool();
}
