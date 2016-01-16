#include "Bundle.h"
#include "NetworkStats.h"
#include "NetworkManager.h"
#include "Channel.h"
#include "PacketSender.h"
#include "MessageHandler.h"
#include "TCPPacket.h"
#include "UDPPacket.h"


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


Bundle::Bundle(Channel *pChannel, ProtocolType pt)
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
	// 这些必须在前面设置
	// 否则中途创建packet可能错误
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

	// 此处对于非固定长度的消息来说需要先设置它的消息长度位为0， 到最后需要填充长度
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

	// 对消息进行跟踪
	if(mpCurrMsgHandler)
	{
		if(bReadyToSend || mNumMessages > 1)
		{
			NetworkStats::getSingleton().trackMessage(NetworkStats::Opt_Send, *mpCurrMsgHandler, mCurrMsgLength);
		}
	}

	// 对于非固定长度的消息来说需要设置它的长度信息
	if(mCurrMsgHandlerLength < 0)
	{
		Packet* pPacket = mpCurrPacket;
		if(mCurrMsgPacketCount > 0)
			pPacket = mPackets[mPackets.size() - mCurrMsgPacketCount];

		mCurrMsgLength -= sizeof(MessageID);
		mCurrMsgLength -= sizeof(MessageLength);

		// 如果消息长度大于等于NETWORK_MESSAGE_MAX_SIZE
		// 使用扩展消息长度机制，向消息长度后面再填充4字节
		// 用于描述更大的长度
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
	// 如果使用了openssl加密通讯则我们保证一个包最大能被Blowfish::BLOCK_SIZE除尽
	// 这样我们在加密一个满载包时不需要额外填充字节
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

	// 如果当前包装不下本次append的数据，将其填充到新包中
	if(fwpos >= mPacketMaxSize)
	{
		mPackets.push_back(mpCurrPacket);
		mCurrMsgPacketCount++;
		newPacket();
		totalsize = 0;
	}

	int32 remainsize = mPacketMaxSize - totalsize;
	int32 taddsize = addsize;

	// 如果当前包剩余空间小于要添加的字节则本次填满此包
	if(remainsize < addsize)
		taddsize = remainsize;
	
	mCurrMsgLength += taddsize;
	return taddsize;
}

Bundle & Bundle::operator<<(uint8 value)
{
	onPacketAppend(sizeof(uint8));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(uint16 value)
{
	onPacketAppend(sizeof(uint16));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(uint32 value)
{
	onPacketAppend(sizeof(uint32));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(uint64 value)
{
	onPacketAppend(sizeof(uint64));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(int8 value)
{
	onPacketAppend(sizeof(int8));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(int16 value)
{
	onPacketAppend(sizeof(int16));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(int32 value)
{
	onPacketAppend(sizeof(int32));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(int64 value)
{
	onPacketAppend(sizeof(int64));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(float value)
{
	onPacketAppend(sizeof(float));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(double value)
{
	onPacketAppend(sizeof(double));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(bool value)
{
	onPacketAppend(sizeof(int8));
	(*mpCurrPacket) << value;
	return *this;
}

Bundle & Bundle::operator<<(const std::string &value)
{
	int32 len = (int32)value.size() + 1; // +1为字符串尾部的0位置
	int32 addtotalsize = 0;

	while(len > 0)
	{
		int32 ilen = onPacketAppend(len, false);
		mpCurrPacket->append(value.c_str() + addtotalsize, ilen);
		addtotalsize += ilen;
		len -= ilen;
	}

	return *this;
}

Bundle & Bundle::operator<<(const char *str)
{
	int32 len = (int32)strlen(str) + 1;  // +1为字符串尾部的0位置
	int32 addtotalsize = 0;

	while(len > 0)
	{
		int32 ilen = onPacketAppend(len, false);
		mpCurrPacket->append(str + addtotalsize, ilen);
		addtotalsize += ilen;
		len -= ilen;
	}

	return *this;
}

Bundle & Bundle::append(Bundle* pBundle)
{
	Assert(pBundle != NULL);
	return append(*pBundle);
}

Bundle & Bundle::append(Bundle& bundle)
{
	Packets::iterator iter = bundle.mPackets.begin();
	for(; iter!=bundle.mPackets.end(); ++iter)
	{
		append((*iter)->data(), (*iter)->length());
	}

	if(bundle.mpCurrPacket == NULL)
		return *this;

	return append(bundle.mpCurrPacket->data(), bundle.mpCurrPacket->length());
}

Bundle & Bundle::append(MemoryStream* s)
{
	Assert(s != NULL);
	return append(*s);
}

Bundle & Bundle::append(MemoryStream& s)
{
	if(s.length() > 0)
		return append(s.data() + s.rpos(), s.length());

	return *this;
}

Bundle & Bundle::append(const uint8 *str, int n)
{
	return assign((char*)str, n);
}

Bundle & Bundle::append(const char *str, int n)
{
	return assign(str, n);
}

Bundle & Bundle::assign(const char *str, int n)
{
	int32 len = (int32)n;
	int32 addtotalsize = 0;

	while(len > 0)
	{
		int32 ilen = onPacketAppend(len, false);
		mpCurrPacket->append((uint8*)(str + addtotalsize), ilen);
		addtotalsize += ilen;
		len -= ilen;
	}

	return *this;
}

Bundle & Bundle::appendBlob(const std::string& str)
{
	return appendBlob((const uint8 *)str.data(), str.size());
}

Bundle & Bundle::appendBlob(const char* str, ArraySize n)
{
	return appendBlob((const uint8 *)str, n);
}

Bundle & Bundle::appendBlob(const uint8 *str, ArraySize n)
{
	(*this) << n;
	return assign((char*)str, n);
}

Bundle & Bundle::operator>>(bool &value)
{
	return outputValue<bool>(value);
}

Bundle & Bundle::operator>>(uint8 &value)
{
	return outputValue<uint8>(value);
}

Bundle & Bundle::operator>>(uint16 &value)
{
	return outputValue<uint16>(value);
}

Bundle & Bundle::operator>>(uint32 &value)
{
	return outputValue<uint32>(value);
}

Bundle & Bundle::operator>>(uint64 &value)
{
	return outputValue<uint64>(value);
}

Bundle & Bundle::operator>>(int8 &value)
{
	return outputValue<int8>(value);
}

Bundle & Bundle::operator>>(int16 &value)
{
	return outputValue<int16>(value);
}

Bundle & Bundle::operator>>(int32 &value)
{
	return outputValue<int32>(value);
}

Bundle & Bundle::operator>>(int64 &value)
{
	return outputValue<int64>(value);
}

Bundle & Bundle::operator>>(float &value)
{
	return outputValue<float>(value);
}

Bundle & Bundle::operator>>(double &value)
{
	return outputValue<double>(value);
}

Bundle & Bundle::operator>>(std::string& value)
{
	Assert(packetsLength() > 0);
	size_t reclaimCount = 0;
	value.clear();

	Packets::iterator iter = mPackets.begin();
	for (; iter != mPackets.end(); ++iter)
	{
		Packet* pPacket = (*iter);

		while (pPacket->length() > 0)
		{
			char c = pPacket->read<char>();
			if (c == 0)
				break;

			value += c;
		}

		if(pPacket->data()[pPacket->rpos() - 1] == 0)
		{
			if(pPacket->length() == 0)
			{
				reclaimPacket(pPacket->isTCPPacket(), pPacket);
				++reclaimCount;
			}

			break;
		}
		else
		{
			Assert(pPacket->length() == 0);
			++reclaimCount;
			reclaimPacket(pPacket->isTCPPacket(), pPacket);
		}
	}

	if(reclaimCount > 0)
		mPackets.erase(mPackets.begin(), mPackets.begin() + reclaimCount);

	return *this;
}

ArraySize Bundle::readBlob(std::string& datas)
{
	datas.clear();

	ArraySize rsize = 0;
	(*this) >> rsize;

	if((int32)rsize > packetsLength())
		return 0;

	size_t reclaimCount = 0;
	datas.reserve(rsize);

	Packets::iterator iter = mPackets.begin();
	for (; iter != mPackets.end(); ++iter)
	{
		Packet* pPacket = (*iter);

		if(pPacket->length() > rsize - datas.size())
		{
			datas.append((char*)pPacket->data() + pPacket->rpos(), rsize - datas.size());
			pPacket->rpos(pPacket->rpos() + rsize - datas.size());
			if(pPacket->length() == 0)
			{
				reclaimPacket(pPacket->isTCPPacket(), pPacket);
				++reclaimCount;
			}

			break;
		}
		else
		{
			datas.append((char*)pPacket->data() + pPacket->rpos(), pPacket->length());
			pPacket->done();
			reclaimPacket(pPacket->isTCPPacket(), pPacket);
			++reclaimCount;
		}
	}

	if(reclaimCount > 0)
		mPackets.erase(mPackets.begin(), mPackets.begin() + reclaimCount);

	return rsize;
}
