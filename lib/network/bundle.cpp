#include "Bundle.h"
#include "network/NetworkStats.h"
#include "network/NetworkManager.h"
#include "network/Channel.h"
#include "helper/profile.h"
#include "network/PacketSender.h"
#include "common/blowfish.h"


static ObjectPool<Bundle> s_ObjPool("Bundle");
ObjectPool<Bundle>& Bundle::ObjPool()
{
	return s_ObjPool;
}

void Bundle::destroyObjPool()
{
	DEBUG_MSG(fmt::format("Bundle::destroyObjPool(): size {}.\n", 
		s_ObjPool.size()));

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

void Bundle::finiMessage(bool isSend)
{
	Assert(mpCurrPacket != NULL);

	mpCurrPacket->setBundle(this);

	if(isSend)
	{
		mCurrMsgPacketCount++;
		mPackets.push_back(mpCurrPacket);
	}

	// ����Ϣ���и���
	if(mpCurrMsgHandler)
	{
		if(isSend || mNumMessages > 1)
		{
			NetworkStats::getSingleton().trackMessage(NetworkStats::Opt_Send, *mpCurrMsgHandler, mCurrMsgLength);
		}
	}

	// �˴����ڷǹ̶����ȵ���Ϣ��˵��Ҫ�����������ճ�����Ϣ
	if(mCurrMsgHandlerLength < 0 || g_packetAlwaysContainLength)
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
			MessageLength1 ex_msg_length = mCurrMsgLength;
			EndianConvert(ex_msg_length);

			MessageLength msgLen = NETWORK_MESSAGE_MAX_SIZE;
			EndianConvert(msgLen);

			memcpy(&pPacket->data()[mCurrMsgLengthPos], (uint8*)&msgLen, sizeof(MessageLength));

			pPacket->insert(mCurrMsgLengthPos + sizeof(MessageLength), (uint8*)&ex_msg_length, sizeof(MessageLength1));
		}
		else
		{
			MessageLength msgLen = (MessageLength)mCurrMsgLength;
			EndianConvert(msgLen);

			memcpy(&pPacket->data()[mCurrMsgLengthPos], (uint8*)&msgLen, sizeof(MessageLength));
		}
	}

	if(isSend)
	{
		mCurrMsgHandlerLength = 0;
		mpCurrPacket = NULL;

		if(g_trace_packet > 0)
			_debugMessages();
	}

	mCurrMsgID = 0;
	mCurrMsgPacketCount = 0;
	mCurrMsgLength = 0;
	mCurrMsgLengthPos = 0;
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

//-------------------------------------------------------------------------------------
void Bundle::_calcPacketMaxSize()
{
	// ���ʹ����openssl����ͨѶ�����Ǳ�֤һ��������ܱ�Blowfish::BLOCK_SIZE����
	// ���������ڼ���һ�����ذ�ʱ����Ҫ��������ֽ�
	if(g_channelExternalEncryptType == 1)
	{
		mPacketMaxSize = mIsTCPPacket ? (TCPPacket::maxBufferSize() - ENCRYPTTION_WASTAGE_SIZE):
			(PACKET_MAX_SIZE_UDP - ENCRYPTTION_WASTAGE_SIZE);

		mPacketMaxSize -= mPacketMaxSize % KBEBlowfish::BLOCK_SIZE;
	}
	else
	{
		mPacketMaxSize = mIsTCPPacket ? TCPPacket::maxBufferSize() : PACKET_MAX_SIZE_UDP;
	}
}

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
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

int Bundle::packetsSize() const
{
	size_t i = mPackets.size();
	if(mpCurrPacket && !mpCurrPacket->empty())
		++i;

	return i;
}

//-------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------
void Bundle::_debugMessages()
{
	if(!mpCurrMsgHandler)
		return;

	Packets packets;
	packets.insert(packets.end(), mPackets.begin(), mPackets.end());
	if(mpCurrPacket)
		packets.push_back(mpCurrPacket);

	MemoryStream* pMemoryStream = MemoryStream::ObjPool().createObject();
	MessageID msgid = 0;
	MessageLength msglen = 0;
	MessageLength1 msglen1 = 0;
	const MessageHandler* pCurrMsgHandler = NULL;

	int state = 0; // 0:��ȡ��ϢID�� 1����ȡ��Ϣ���ȣ� 2����ȡ��Ϣ��չ����, 3:��ȡ����

	for(Packets::iterator iter = packets.begin(); iter != packets.end(); iter++)
	{
		Packet* pPacket = (*iter);
		if(pPacket->length() == 0)
			continue;

		size_t rpos = pPacket->rpos();
		size_t wpos = pPacket->wpos();

		while(pPacket->length() > 0)
		{
			if(state == 0)
			{
				// һЩsendto�����İ�����, �����Ҳ����Ҫ׷��
				if(pPacket->length() < sizeof(MessageID))
				{
					pPacket->done();
					continue;
				}

				(*pPacket) >> msgid;
				(*pMemoryStream) << msgid;
				state = 1;
				continue;
			}
			else if(state == 1)
			{
				if(!mpCurrMsgHandler || !mpCurrMsgHandler->pMessageHandlers)
				{
					pPacket->done();
					continue;
				}

				pCurrMsgHandler = mpCurrMsgHandler->pMessageHandlers->find(msgid);

				// һЩsendto�����İ������Ҳ���MsgHandler, �����Ҳ����Ҫ׷��
				if(!pCurrMsgHandler)
				{
					pPacket->done();
					continue;
				}

				if(pCurrMsgHandler->msgLen == NETWORK_VARIABLE_MESSAGE || g_packetAlwaysContainLength)
				{
					(*pPacket) >> msglen;
					(*pMemoryStream) << msglen;

					if(msglen == NETWORK_MESSAGE_MAX_SIZE)
						state = 2;
					else
						state = 3;
				}
				else
				{
					msglen = pCurrMsgHandler->msgLen;
					(*pMemoryStream) << msglen;
					state = 3;
				}

				continue;
			}
			else if(state == 2)
			{
				(*pPacket) >> msglen1;
				(*pMemoryStream) << msglen1;
				state = 3;
				continue;
			}
			else if(state == 3)
			{
				MessageLength1 totallen = msglen1 > 0 ? msglen1 : msglen;
				
				if(pPacket->length() >= totallen - pMemoryStream->length())
				{
					MessageLength1 len  = totallen - pMemoryStream->length();
					pMemoryStream->append(pPacket->data() + pPacket->rpos(), len);
					pPacket->rpos(pPacket->rpos() + len);
				}
				else
				{
					pMemoryStream->append(*static_cast<MemoryStream*>(pPacket));
					pPacket->done();
				}

				if(pMemoryStream->length() == totallen)
				{
					state = 0;
					msglen1 = 0;
					msglen = 0;
					msgid = 0;

					TRACE_MESSAGE_PACKET(false, pMemoryStream, pCurrMsgHandler, pMemoryStream->length(), 
						(mpChannel != NULL ? mpChannel->c_str() : "None"));

					pMemoryStream->clear(false);
					continue;
				}
			}
		};

		pPacket->rpos(rpos);
		pPacket->wpos(wpos);
	}

	MemoryStream::ObjPool().reclaimObject(pMemoryStream);
}
