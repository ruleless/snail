#ifndef KBE_NETWORK_COMMON_H
#define KBE_NETWORK_COMMON_H

// common include
#include "common/common.h"
#include "helper/debug_option.h"

const uint32 BROADCAST = 0xFFFFFFFF;
const uint32 LOCALHOST = 0x0100007F;

extern uint32 gListenQ;

// ��Ϣ��ID
typedef uint16	MessageID;

// ��Ϣ���ȣ�Ŀǰ������2�֣�Ĭ����Ϣ�������MessageLength
// �����������ʱ��Ҫ��չ���ȣ��ײ�ʹ��MessageLength1
typedef uint16	MessageLength;		// ���65535
typedef uint32	MessageLength1;		// ���4294967295

typedef int32	ChannelID;
const ChannelID CHANNEL_ID_NULL = 0;

// ͨ����ʱʱ��
extern float gChannelInternalTimeout;
extern float gChannelExternalTimeout;

// ͨ�����ͳ�ʱ����
extern uint32 g_intReSendInterval;
extern uint32 g_intReSendRetries;
extern uint32 g_extReSendInterval;
extern uint32 g_extReSendRetries;

// �ⲿͨ���������
extern int8 g_channelExternalEncryptType;

// listen�����������ֵ
extern uint32 gList;

// ���ܶ���洢����Ϣռ���ֽ�(����+���)
#define ENCRYPTTION_WASTAGE_SIZE			(1 + 7)

#define PACKET_MAX_SIZE						1500
#ifndef PACKET_MAX_SIZE_TCP
#define PACKET_MAX_SIZE_TCP					1460
#endif
#define PACKET_MAX_SIZE_UDP					1472

typedef uint16								PacketLength;				// ���65535
#define PACKET_LENGTH_SIZE					sizeof(PacketLength)

#define NETWORK_MESSAGE_MAX_SIZE			65535
#define NETWORK_MESSAGE_MAX_SIZE1			4294967295

// ��Ϸ���ݿ��ð���С
#define GAME_PACKET_MAX_SIZE_TCP			PACKET_MAX_SIZE_TCP - sizeof(MessageID) - \
											sizeof(MessageLength) - ENCRYPTTION_WASTAGE_SIZE

/** kbe machine�˿� */
#define KBE_PORT_START						20000
#define KBE_MACHINE_BRAODCAST_SEND_PORT		KBE_PORT_START + 86			// machine���չ㲥�Ķ˿�
#define KBE_PORT_BROADCAST_DISCOVERY		KBE_PORT_START + 87
#define KBE_MACHINE_TCP_PORT				KBE_PORT_START + 88

#define KBE_INTERFACES_TCP_PORT				30099

/*
	������Ϣ���ͣ� �������߱䳤��
	�����Ҫ�Զ��峤������NETWORK_INTERFACE_DECLARE_BEGIN������ʱ���볤�ȼ��ɡ�
*/
#ifndef NETWORK_FIXED_MESSAGE
#define NETWORK_FIXED_MESSAGE 0
#endif

#ifndef NETWORK_VARIABLE_MESSAGE
#define NETWORK_VARIABLE_MESSAGE -1
#endif

// ������Ϣ���
enum NETWORK_MESSAGE_TYPE
{
	NETWORK_MESSAGE_TYPE_COMPONENT = 0,	// �����Ϣ
	NETWORK_MESSAGE_TYPE_ENTITY = 1,	// entity��Ϣ
};

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

INLINE
const char * reasonToString(EReason reason)
{
	const char * reasons[] =
	{
		"REASON_SUCCESS",
		"REASON_TIMER_EXPIRED",
		"REASON_NO_SUCH_PORT",
		"REASON_GENERAL_NETWORK",
		"REASON_CORRUPTED_PACKET",
		"REASON_NONEXISTENT_ENTRY",
		"REASON_WINDOW_OVERFLOW",
		"REASON_INACTIVITY",
		"REASON_RESOURCE_UNAVAILABLE",
		"REASON_CLIENT_DISCONNECTED",
		"REASON_TRANSMIT_QUEUE_FULL",
		"REASON_CHANNEL_LOST",
		"REASON_SHUTTING_DOWN",
		"REASON_WEBSOCKET_ERROR",
		"REASON_CHANNEL_CONDEMN"
	};

	unsigned int index = -reason;

	if (index < sizeof(reasons)/sizeof(reasons[0]))
	{
		return reasons[index];
	}

	return "REASON_UNKNOWN";
}

#define SEND_BUNDLE_COMMON(SND_FUNC, BUNDLE)																\
	BUNDLE.finiMessage();																					\
																											\
	Bundle::Packets::iterator iter = BUNDLE.packets().begin();										\
	for (; iter != BUNDLE.packets().end(); ++iter)															\
	{																										\
		Packet* pPacket = (*iter);																			\
		int retries = 0;																					\
		EReason reason;																						\
		pPacket->sentSize = 0;																				\
																											\
		while(true)																							\
		{																									\
			++retries;																						\
			int slen = SND_FUNC;																			\
																											\
			if(slen > 0)																					\
				pPacket->sentSize += slen;																	\
																											\
			if(pPacket->sentSize != pPacket->length())														\
			{																								\
				reason = PacketSender::checkSocketErrors(&ep);												\
				/* ������ͳ��ִ�����ô���ǿ��Լ�������һ�Σ� ����60���˳�	*/								\
				if (reason == Reason_NoSuchPort && retries <= 3)											\
				{																							\
					continue;																				\
				}																							\
																											\
				/* ���ϵͳ���ͻ����Ѿ����ˣ������ǵȴ�10ms	*/												\
				if ((reason == Reason_ResourceUnavailable || reason == Reason_GeneralNetwork)				\
																					&& retries <= 60)		\
				{																							\
																											\
					ep.waitSend();																			\
					continue;																				\
				}																							\
																											\
				if(retries > 60 && reason != Reason_Success)												\
				{																							\
					break;																					\
				}																							\
			}																								\
			else																							\
			{																								\
				break;																						\
			}																								\
		}																									\
																											\
	}																										\
																											\
	BUNDLE.clearPackets();																					\
																											\


#define SEND_BUNDLE(ENDPOINT, BUNDLE)																		\
{																											\
	EndPoint& ep = ENDPOINT;																				\
	SEND_BUNDLE_COMMON(ENDPOINT.send(pPacket->data() + pPacket->sentSize,									\
	pPacket->length() - pPacket->sentSize), BUNDLE);														\
}																											\


#define SENDTO_BUNDLE(ENDPOINT, ADDR, PORT, BUNDLE)															\
{																											\
	EndPoint& ep = ENDPOINT;																				\
	SEND_BUNDLE_COMMON(ENDPOINT.sendto(pPacket->data() + pPacket->sentSize,									\
	pPacket->length() - pPacket->sentSize, PORT, ADDR), BUNDLE);											\
}																											\

class Packet;
extern Packet* mallocPacket(bool isTCPPacket);
extern void reclaimPacket(bool isTCPPacket, Packet *pPacket);

extern void destroyObjPool();

// network stats
extern uint64						g_numPacketsSent;
extern uint64						g_numPacketsReceived;
extern uint64						g_numBytesSent;
extern uint64						g_numBytesReceived;

// �����մ������
extern uint32						g_receiveWindowMessagesOverflowCritical;
extern uint32						g_intReceiveWindowMessagesOverflow;
extern uint32						g_extReceiveWindowMessagesOverflow;
extern uint32						g_intReceiveWindowBytesOverflow;
extern uint32						g_extReceiveWindowBytesOverflow;

extern uint32						gSendWindowMessagesOverflowCritical;
extern uint32						gIntSendWindowMessagesOverflow;
extern uint32						gExtSendWindowMessagesOverflow;
extern uint32						g_intSendWindowBytesOverflow;
extern uint32						g_extSendWindowBytesOverflow;

void finalise(void);

#endif // KBE_NETWORK_COMMON_H
