#ifndef __NETWORKDEF_H__
#define __NETWORKDEF_H__

#include "common.h"

const uint32 BROADCAST = 0xFFFFFFFF;
const uint32 LOCALHOST = 0x0100007F;

// 消息的ID
typedef uint16 MessageID;

// 消息长度，目前长度有2种，默认消息长度最大MessageLength
// 当超过这个数时需要扩展长度，底层使用MessageLength1
typedef uint16 MessageLength; // 最大65535
typedef uint32 MessageLength1; // 最大4294967295

typedef int32 ChannelID;
const ChannelID CHANNEL_ID_NULL = 0;

typedef uint16 PacketLength; // 最大65535

// 加密额外存储的信息占用字节(长度+填充)
#define ENCRYPTTION_WASTAGE_SIZE (1 + 7)

#define PACKET_MAX_SIZE 1500
#ifndef PACKET_MAX_SIZE_TCP
#define PACKET_MAX_SIZE_TCP 1460
#endif
#define PACKET_MAX_SIZE_UDP 1472

#define NETWORK_MESSAGE_MAX_SIZE 65535
#define NETWORK_MESSAGE_MAX_SIZE1 4294967295

// 定长消息
#ifndef NETWORK_FIXED_MESSAGE
#define NETWORK_FIXED_MESSAGE 0
#endif

// 变长消息
#ifndef NETWORK_VARIABLE_MESSAGE
#define NETWORK_VARIABLE_MESSAGE -1
#endif

enum ProtocolType
{
	Protocol_TCP = 0,
	Protocol_UDP = 1,
};

enum EReason
{
	Reason_Success = 0,
	Reason_TimerExpired = -1,
	Reason_NoSuchPort = -2,
	Reason_GeneralNetwork = -3,
	Reason_CorruptedPacket = -4,
	Reason_NonExistentEntry = -5,
	Reason_WindowOverflow = -6,
	Reason_Inactivity = -7,
	Reason_ResourceUnavailable = -8,
	Reason_ClientDisconnected = -9,
	Reason_TransmitQueueFull = -10,
	Reason_ChannelLost = -11,
	Reason_ShuttingDown = -12,
	Reason_WebSocketError = -13,
	Reason_ChannelCondemn = -14,
};

// 已完成连接队列的最大容量
extern uint32 gListenQ;

// 通道超时时间
extern float gChannelInternalTimeout;
extern float gChannelExternalTimeout;

// 通道发送超时重试
extern uint32 gIntReSendInterval;
extern uint32 gIntReSendRetries;
extern uint32 gExtReSendInterval;
extern uint32 gExtReSendRetries;

// 外部通道加密类别
extern int8 gChannelExternalEncryptType;

// listen监听队列最大值
extern uint32 gList;

// network stats
extern uint64 gNumPacketsSent;
extern uint64 gNumPacketsReceived;
extern uint64 gNumBytesSent;
extern uint64 gNumBytesReceived;

// 包接收窗口溢出参数
extern uint32 gReceiveWindowMessagesOverflowCritical;
extern uint32 gIntReceiveWindowMessagesOverflow;
extern uint32 gExtReceiveWindowMessagesOverflow;
extern uint32 gIntReceiveWindowBytesOverflow;
extern uint32 gExtReceiveWindowBytesOverflow;

// 包发送窗口溢出参数
extern uint32 gSendWindowMessagesOverflowCritical;
extern uint32 gIntSendWindowMessagesOverflow;
extern uint32 gExtSendWindowMessagesOverflow;
extern uint32 gIntSendWindowBytesOverflow;
extern uint32 gExtSendWindowBytesOverflow;

class Packet;
extern Packet* mallocPacket(bool isTCPPacket);
extern void reclaimPacket(bool isTCPPacket, Packet *pPacket);

extern void destroyObjPool();
extern void finalise(void);

#endif // __NETWORKDEF_H__
