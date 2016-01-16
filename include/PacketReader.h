#ifndef __PACKETREADER_H__
#define __PACKETREADER_H__

#include "MemoryStream.h"
#include "NetworkDef.h"

class Channel;
class MessageHandlers;

class PacketReader
{
  public:
	enum EPacketReaderType
	{
		PacketReaderType_Socket = 0,
		PacketReaderType_WebSocket,
	};

	PacketReader(Channel* pChannel);
	virtual ~PacketReader();

	virtual void reset();
	
	virtual void processMessages(MessageHandlers* pMsgHandlers, Packet* pPacket);
	
	MessageID currMsgID() const { return mCurrMsgID; }
	MessageLength currMsgLen() const { return mCurrMsgLen; }
	
	void currMsgID(MessageID id) { mCurrMsgID = id; }
	void currMsgLen(MessageLength len) { mCurrMsgLen = len; }

	virtual PacketReader::EPacketReaderType type()const { return PacketReaderType_Socket; }
  protected:
	enum EFragmentDataTypes
	{
		FragmentData_Unknown,
		FragmentData_MessageID,
		FragmentData_MessageLength,
		FragmentData_MessageLength1,
		FragmentData_MessageBody,
	};
	
	virtual void writeFragmentMessage(EFragmentDataTypes fragmentDatasFlag, Packet* pPacket, uint32 datasize);
	virtual void mergeFragmentMessage(Packet* pPacket);
  protected:
	MessageID mCurrMsgID;
	MessageLength1 mCurrMsgLen;
	MemoryStream *mpFragmentStream;

	EFragmentDataTypes mFragmentDatasFlag;
	uint8 *mpFragmentDatas;
	uint32 mFragmentDatasWpos;
	uint32 mFragmentDatasRemain;
	
	Channel *mpChannel;
};

#endif // __PACKETREADER_H__
