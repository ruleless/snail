#ifndef __NETWORKDEF_H__
#define __NETWORKDEF_H__

#include "common.h"

const uint32 BROADCAST = 0xFFFFFFFF;
const uint32 LOCALHOST = 0x0100007F;

// ��Ϣ��ID
typedef uint16 MessageID;

// ��Ϣ���ȣ�Ŀǰ������2�֣�Ĭ����Ϣ�������MessageLength
// �����������ʱ��Ҫ��չ���ȣ��ײ�ʹ��MessageLength1
typedef uint16 MessageLength; // ���65535
typedef uint32 MessageLength1; // ���4294967295

typedef int32 ChannelID;
const ChannelID CHANNEL_ID_NULL = 0;

typedef uint16 PacketLength; // ���65535

// ���ܶ���洢����Ϣռ���ֽ�(����+���)
#define ENCRYPTTION_WASTAGE_SIZE (1 + 7)

#define PACKET_MAX_SIZE 1500
#ifndef PACKET_MAX_SIZE_TCP
#define PACKET_MAX_SIZE_TCP 1460
#endif
#define PACKET_MAX_SIZE_UDP 1472

#define NETWORK_MESSAGE_MAX_SIZE 65535
#define NETWORK_MESSAGE_MAX_SIZE1 4294967295

// ������Ϣ
#ifndef NETWORK_FIXED_MESSAGE
#define NETWORK_FIXED_MESSAGE 0
#endif

// �䳤��Ϣ
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

// ��������Ӷ��е��������
extern uint32 gListenQ;

// ͨ����ʱʱ��
extern float gChannelInternalTimeout;
extern float gChannelExternalTimeout;

// ͨ�����ͳ�ʱ����
extern uint32 gIntReSendInterval;
extern uint32 gIntReSendRetries;
extern uint32 gExtReSendInterval;
extern uint32 gExtReSendRetries;

// �ⲿͨ���������
extern int8 gChannelExternalEncryptType;

// listen�����������ֵ
extern uint32 gList;

// network stats
extern uint64 gNumPacketsSent;
extern uint64 gNumPacketsReceived;
extern uint64 gNumBytesSent;
extern uint64 gNumBytesReceived;

// �����մ����������
extern uint32 gReceiveWindowMessagesOverflowCritical;
extern uint32 gIntReceiveWindowMessagesOverflow;
extern uint32 gExtReceiveWindowMessagesOverflow;
extern uint32 gIntReceiveWindowBytesOverflow;
extern uint32 gExtReceiveWindowBytesOverflow;

// �����ʹ����������
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
