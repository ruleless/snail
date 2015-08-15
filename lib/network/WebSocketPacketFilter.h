#ifndef __WEBSOCKETPACKETFILTER_H__
#define __WEBSOCKETPACKETFILTER_H__

#include "network/PacketFilter.h"
#include "network/websocket_protocol.h"

class TCPPacket;

class WebSocketPacketFilter : public PacketFilter
{
public:
	WebSocketPacketFilter(Channel* pChannel);
	virtual ~WebSocketPacketFilter();

	virtual EReason send(Channel * pChannel, PacketSender& sender, Packet * pPacket);
	virtual EReason recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket);
protected:
	void reset();
protected:
	enum EFragmentDataTypes
	{
		FragmentDataTypes_MessageHRead,
		FragmentDataTypes_MessageDatas,
	};

	int32 pFragmentDatasRemain_;
	EFragmentDataTypes fragmentDatasFlag_;

	uint8 msg_opcode_;
	uint8 msg_fin_;
	uint8 msg_masked_;
	uint32 msg_mask_;
	int32 msg_length_field_;
	uint64 msg_payload_length_;
	WebSocketProtocol::FrameType msg_frameType_;

	Channel *pChannel_;

	TCPPacket *pTCPPacket_;
};

#endif // __WEBSOCKETPACKETFILTER_H__
