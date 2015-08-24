#include "Bundle.h"
#include "network/NetworkStats.h"
#include "network/NetworkManager.h"
#include "network/Channel.h"
#include "network/PacketSender.h"
#include "network/MessageHandler.h"


static ObjectPool<Bundle> s_ObjPool("Bundle");
ObjectPool<Bundle>& Bundle::ObjPool()
{
	return s_ObjPool;
}

void Bundle::destroyObjPool()
{
// 	DEBUG_MSG(fmt::format("Bundle::destroyObjPool(): size {}.\n", 
// 		s_ObjPool.size()));

	s_ObjPool.destroy();
}

void Bundle::onReclaimObject()
{
	clear(true);
}

size_t Bundle::getPoolObjectBytes()
{
	size_t bytes = sizeof(mpCurrMsgHandler) + sizeof(mIsTCPPacket) + 
		sizeof(mCurrMsgLengthPos) + sizeof(mCurrMsgHandlerLength) + sizeof(mCurrMsgLength) + 
		sizeof(mCurrMsgPacketCount) + sizeof(mCurrMsgID) + sizeof(mNumMessages) + sizeof(mpChannel)
		+ (mPackets.size() * sizeof(Packet*));

	return bytes;
}


Bundle::Bundle(Channel * pChannel, ProtocolType pt)
:mpChannel(pChannel)
,mNumMessages(0)
,mpCurrPacket(NULL)
,mCurrMsgID(0)
,mCurrMsgPacketCount(0)
,mCurrMsgLength(0)
,mCurrMsgHandlerLength(0)
,mCurrMsgLengthPos(0)
,mPackets()
,mIsTCPPacket(pt == Protocol_TCP)
,mPacketMaxSize(0)
,mpCurrMsgHandler(NULL)
{
	_calcPacketMaxSize();
	 newPacket();
}

Bundle::Bundle(const Bundle& bundle)
{
	// ��Щ������ǰ������
	// ������;����packet���ܴ���
	mIsTCPPacket = bundle.mIsTCPPacket;
	mpChannel = bundle.mpChannel;
	mpCurrMsgHandler = bundle.mpCurrMsgHandler;
	mCurrMsgID = bundle.mCurrMsgID;

	Packets::const_iterator iter = bundle.mPackets.begin();
	for (; iter != bundle.mPackets.end(); ++iter)
	{
		newPacket();
		mpCurrPacket->append(*static_cast<MemoryStream *>((*iter)));
		mPackets.push_back(mpCurrPacket);
	}

	mpCurrPacket = NULL;
	if(bundle.mpCurrPacket)
	{
		newPacket();
		mpCurrPacket->append(*static_cast<MemoryStream*>(bundle.mpCurrPacket));
	}

	mNumMessages = bundle.mNumMessages;
	mCurrMsgPacketCount = bundle.mCurrMsgPacketCount;
	mCurrMsgLength = bundle.mCurrMsgLength;
	mCurrMsgHandlerLength = bundle.mCurrMsgHandlerLength;
	mCurrMsgLengthPos = bundle.mCurrMsgLengthPos;
	_calcPacketMaxSize();
}

Bundle::~Bundle()
{
	clear(false);
}

void Bundle::newMessage(const MessageHandler& msgHandler)
{
	mpCurrMsgHandler = &msgHandler;

	if(mpCurrPacket == NULL)
		this->newPacket();

	finiMessage(false);
	Assert(mpCurrPacket != NULL);

	(*this) << msgHandler.msgID;
	mpCurrPacket->setMessageID(msgHandler.msgID);

	// �˴����ڷǹ̶����ȵ���Ϣ��˵��Ҫ������������Ϣ����λΪ0�� �������Ҫ��䳤��
	if(msgHandler.msgLen == NETWORK_VARIABLE_MESSAGE)
	{
		MessageLength msglen = 0;
		mCurrMsgLengthPos = mpCurrPacket->wpos();
		(*this) << msglen;
	}

	++mNumMessages;
	mCurrMsgID = msgHandler.msgID;
	mCurrMsgPacketCount = 0;
	mCurrMsgHandlerLength = msgHandler.msgLen;
}

void Bundle::finiMessage(bool bReadyToSend)
{
	Assert(mpCurrPacket != NULL);

	mpCurrPacket->setBundle(this);

	if(bReadyToSend)
	{
		mCurrMsgPacketCount++;
		mPackets.push_back(mpCurrPacket);
	}

	// ����Ϣ���и���
	if(mpCurrMsgHandler)
	{
		if(bReadyToSend || mNumMessages > 1)
		{
			NetworkStats::getSingleton().trackMessage(NetworkStats::Opt_Send, *mpCurrMsgHandler, mCurrMsgLength);
		}
	}

	// ���ڷǹ̶����ȵ���Ϣ��˵��Ҫ�������ĳ�����Ϣ
	if(mCurrMsgHandlerLength < 0)
	{
		Packet* pPacket = mpCurrPacket;
		if(mCurrMsgPacketCount > 0)
			pPacket = mPackets[mPackets.size() - mCurrMsgPacketCount];

		mCurrMsgLength -= sizeof(MessageID);
		mCurrMsgLength -= sizeof(MessageLength);

		// �����Ϣ���ȴ��ڵ���NETWORK_MESSAGE_MAX_SIZE
		// ʹ����չ��Ϣ���Ȼ��ƣ�����Ϣ���Ⱥ��������4�ֽ�
		// ������������ĳ���
		if(mCurrMsgLength >= NETWORK_MESSAGE_MAX_SIZE)
		{
			MessageLength1 exMsgLength = mCurrMsgLength;
			EndianConvert(exMsgLength);

			MessageLength msgLen = NETWORK_MESSAGE_MAX_SIZE;
			EndianConvert(msgLen);

			memcpy(&pPacket->data()[mCurrMsgLengthPos], (uint8*)&msgLen, sizeof(MessageLength));

			pPacket->insert(mCurrMsgLengthPos + sizeof(MessageLength), (uint8*)&exMsgLength, sizeof(MessageLength1));
		}
		else
		{
			MessageLength msgLen = (MessageLength)mCurrMsgLength;
			EndianConvert(msgLen);

			memcpy(&pPacket->data()[mCurrMsgLengthPos], (uint8*)&msgLen, sizeof(MessageLength));
		}
	}

	if(bReadyToSend)
	{
		mCurrMsgHandlerLength = 0;
		mpCurrPacket = NULL;
	}

	mCurrMsgID = 0;
	mCurrMsgPacketCount = 0;
	mCurrMsgLength = 0;
	mCurrMsgLengthPos = 0;
}

void Bundle::clearPackets()
{
	if(mpCurrPacket != NULL)
	{
		mPackets.push_back(mpCurrPacket);
		mpCurrPacket = NULL;
	}

	Packets::iterator iter = mPackets.begin();
	for (; iter != mPackets.end(); ++iter)
	{
		reclaimPacket(mIsTCPPacket, (*iter));
	}

	mPackets.clear();
}

void Bundle::clear(bool isRecl)
{
	if(mpCurrPacket != NULL)
	{
		mPackets.push_back(mpCurrPacket);
		mpCurrPacket = NULL;
	}

	Packets::iterator iter = mPackets.begin();
	for (; iter != mPackets.end(); ++iter)
	{
		if(!isRecl)
		{
			delete (*iter);
		}
		else
		{
			reclaimPacket(mIsTCPPacket, (*iter));
		}
	}

	mPackets.clear();

	mpChannel = NULL;
	mNumMessages = 0;

	mCurrMsgID = 0;
	mCurrMsgPacketCount = 0;
	mCurrMsgLength = 0;
	mCurrMsgLengthPos = 0;
	mCurrMsgHandlerLength = 0;
	mpCurrMsgHandler = NULL;
	_calcPacketMaxSize();
}

Packet* Bundle::newPacket()
{
	mpCurrPacket = mallocPacket(mIsTCPPacket);
	mpCurrPacket->setBundle(this);
	return mpCurrPacket;
}

void Bundle::finiCurrPacket()
{ 
	mPackets.push_back(mpCurrPacket); 
	mpCurrPacket = NULL; 
}

int32 Bundle::packetsLength(bool calccurr)
{
	int32 len = 0;

	Packets::iterator iter = mPackets.begin();
	for (; iter != mPackets.end(); ++iter)
	{
		len += (*iter)->length();
	}

	if(calccurr && mpCurrPacket)
		len += mpCurrPacket->length();

	return len;
}

int Bundle::packetsSize() const
{
	size_t i = mPackets.size();
	if(mpCurrPacket && !mpCurrPacket->empty())
		++i;

	return i;
}

void Bundle::_calcPacketMaxSize()
{
	// ���ʹ����openssl����ͨѶ�����Ǳ�֤һ��������ܱ�Blowfish::BLOCK_SIZE����
	// ���������ڼ���һ�����ذ�ʱ����Ҫ��������ֽ�
#ifdef USE_OPENSSL
	if(g_channelExternalEncryptType == 1)
	{
		mPacketMaxSize = mIsTCPPacket ? (TCPPacket::maxBufferSize() - ENCRYPTTION_WASTAGE_SIZE):
			(PACKET_MAX_SIZE_UDP - ENCRYPTTION_WASTAGE_SIZE);

		mPacketMaxSize -= mPacketMaxSize % Blowfish::BLOCK_SIZE;
	}
	else
	{
		mPacketMaxSize = mIsTCPPacket ? TCPPacket::maxBufferSize() : PACKET_MAX_SIZE_UDP;
	}
#else
	mPacketMaxSize = mIsTCPPacket ? TCPPacket::maxBufferSize() : PACKET_MAX_SIZE_UDP;
#endif
}

int32 Bundle::onPacketAppend(int32 addsize, bool inseparable)
{
	if(mpCurrPacket == NULL)
	{
		newPacket();
	}

	int32 totalsize = (int32)mpCurrPacket->length();
	int32 fwpos = (int32)mpCurrPacket->wpos();

	if(inseparable)
		fwpos += addsize;

	// �����ǰ��װ���±���append�����ݣ�������䵽�°���
	if(fwpos >= mPacketMaxSize)
	{
		mPackets.push_back(mpCurrPacket);
		mCurrMsgPacketCount++;
		newPacket();
		totalsize = 0;
	}

	int32 remainsize = mPacketMaxSize - totalsize;
	int32 taddsize = addsize;

	// �����ǰ��ʣ��ռ�С��Ҫ��ӵ��ֽ��򱾴������˰�
	if(remainsize < addsize)
		taddsize = remainsize;
	
	mCurrMsgLength += taddsize;
	return taddsize;
}