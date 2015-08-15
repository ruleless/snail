#ifndef __BUNDLE_H__
#define __BUNDLE_H__

#include "common/common.h"
#include "common/Timer.h"
#include "common/ObjectPool.h"
#include "helper/debug_helper.h"
#include "network/Address.h"
#include "network/EventDispatcher.h"
#include "network/EndPoint.h"
#include "network/NetworkDef.h"
#include "network/TCPPacket.h"
#include "network/UDPPacket.h"
#include "network/interface_defs.h"

class NetworkManager;
class Channel;

#define PACKET_OUT_VALUE(v, expectSize)																		\
	Assert(packetsLength() >= (int32)expectSize);														\
																											\
	size_t currSize = 0;																					\
	size_t reclaimCount = 0;																				\
																											\
	Packets::iterator iter = mPackets.begin();																\
	for (; iter != mPackets.end(); ++iter)																	\
	{																										\
		Packet* pPacket = (*iter);																			\
		size_t remainSize = (size_t)expectSize - currSize;													\
																											\
		if(pPacket->length() >= remainSize)																	\
		{																									\
			memcpy(((uint8*)&v) + currSize, pPacket->data() + pPacket->rpos(), remainSize);					\
			pPacket->rpos(pPacket->rpos() + remainSize);													\
																											\
			if(pPacket->length() == 0)																		\
			{																								\
				reclaimPacket(pPacket->isTCPPacket(), pPacket);											\
				++reclaimCount;																				\
			}																								\
																											\
			break;																							\
		}																									\
		else																								\
		{																									\
			memcpy(((uint8*)&v) + currSize, pPacket->data() + pPacket->rpos(), pPacket->length());			\
			currSize += pPacket->length();																	\
			pPacket->done();																				\
			reclaimPacket(pPacket->isTCPPacket(), pPacket);												\
			++reclaimCount;																					\
		}																									\
	}																										\
																											\
	if(reclaimCount > 0)																					\
		mPackets.erase(mPackets.begin(), mPackets.begin() + reclaimCount);									\
																											\
	return *this;																							\

class Bundle : public PoolObject
{
public:
	static ObjectPool<Bundle>& ObjPool();
	static void destroyObjPool();
	virtual void onReclaimObject();
	virtual size_t getPoolObjectBytes();

	typedef std::vector<Packet *> Packets;
	
	Bundle(Channel * pChannel = NULL, ProtocolType pt = Protocol_TCP);
	Bundle(const Bundle& bundle);
	virtual ~Bundle();
	
	void newMessage(const MessageHandler& msgHandler);
	void finiMessage(bool isSend = true);

	Packet* newPacket();
	void finiCurrPacket();

	void clearPackets();
	void clear(bool isRecl);

	// 计算所有包包括当前还未写完的包的总长度
	int32 packetsLength(bool calccurr = true);
	int packetsSize() const;

	MessageLength currMsgLength() const { return mCurrMsgLength; }
	
	void pCurrMsgHandler(const MessageHandler* pMsgHandler) { mpCurrMsgHandler = pMsgHandler; }
	const MessageHandler* pCurrMsgHandler() const { return mpCurrMsgHandler; }

	bool isTCPPacket() const { return mIsTCPPacket; }
	void isTCPPacket(bool v) { mIsTCPPacket = v; }

	bool empty() const { return packetsSize() == 0; }
	Packets& packets() { return mPackets; }
	Packet* pCurrPacket() const { return mpCurrPacket; }
	void pCurrPacket(Packet* p) { mpCurrPacket = p; }	
	
	MessageID getMessageID() const { return mCurrMsgID; }
	int32 numMessages() const { return mNumMessages; }
protected:
	void _calcPacketMaxSize();
	int32 onPacketAppend(int32 addsize, bool inseparable = true);

	void _debugMessages();
public:
    Bundle &operator<<(uint8 value)
    {
		onPacketAppend(sizeof(uint8));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(uint16 value)
    {
		onPacketAppend(sizeof(uint16));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(uint32 value)
    {
		onPacketAppend(sizeof(uint32));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(uint64 value)
    {
		onPacketAppend(sizeof(uint64));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(int8 value)
    {
		onPacketAppend(sizeof(int8));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(int16 value)
    {
		onPacketAppend(sizeof(int16));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(int32 value)
    {
		onPacketAppend(sizeof(int32));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(int64 value)
    {
		onPacketAppend(sizeof(int64));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(float value)
    {
		onPacketAppend(sizeof(float));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(double value)
    {
		onPacketAppend(sizeof(double));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(COMPONENT_TYPE value)
    {
		onPacketAppend(sizeof(int32));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(ENTITY_MAILBOX_TYPE value)
    {
		onPacketAppend(sizeof(int32));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(bool value)
    {
		onPacketAppend(sizeof(int8));
        (*mpCurrPacket) << value;
        return *this;
    }

    Bundle &operator<<(const std::string &value)
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
	
    Bundle &operator<<(const char *str)
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
    
	Bundle &append(Bundle* pBundle)
	{
		Assert(pBundle != NULL);
		return append(*pBundle);
	}

	Bundle &append(Bundle& bundle)
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

	Bundle &append(MemoryStream* s)
	{
		Assert(s != NULL);
		return append(*s);
	}

	Bundle &append(MemoryStream& s)
	{
		if(s.length() > 0)
			return append(s.data() + s.rpos(), s.length());

		return *this;
	}

	Bundle &appendBlob(const std::string& str)
	{
		return appendBlob((const uint8 *)str.data(), str.size());
	}

	Bundle &appendBlob(const char* str, ArraySize n)
	{
		return appendBlob((const uint8 *)str, n);
	}

	Bundle &appendBlob(const uint8 *str, ArraySize n)
	{
		(*this) << n;
		return assign((char*)str, n);
	}

	Bundle &append(const uint8 *str, int n)
	{
		return assign((char*)str, n);
	}

	Bundle &append(const char *str, int n)
	{
		return assign(str, n);
	}

	Bundle &assign(const char *str, int n)
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

    Bundle &operator>>(bool &value)
    {
        PACKET_OUT_VALUE(value, sizeof(bool));
    }

    Bundle &operator>>(uint8 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(uint8));
    }

    Bundle &operator>>(uint16 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(uint16));
    }

    Bundle &operator>>(uint32 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(uint32));
    }

    Bundle &operator>>(uint64 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(uint64));
    }

    Bundle &operator>>(int8 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(int8));
    }

    Bundle &operator>>(int16 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(int16));
    }

    Bundle &operator>>(int32 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(int32));
    }

    Bundle &operator>>(int64 &value)
    {
        PACKET_OUT_VALUE(value, sizeof(int64));
    }

    Bundle &operator>>(float &value)
    {
        PACKET_OUT_VALUE(value, sizeof(float));
    }

    Bundle &operator>>(double &value)
    {
        PACKET_OUT_VALUE(value, sizeof(double));
    }

    Bundle &operator>>(std::string& value)
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

	ArraySize readBlob(std::string& datas)
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
private:
	Channel *mpChannel;
	int32 mNumMessages;
	
	Packet *mpCurrPacket;
	MessageID mCurrMsgID;
	uint32 mCurrMsgPacketCount;
	MessageLength1 mCurrMsgLength;	
	int32 mCurrMsgHandlerLength;
	size_t mCurrMsgLengthPos;

	Packets mPackets;
	
	bool mIsTCPPacket;
	int32 mPacketMaxSize;

	const MessageHandler* mpCurrMsgHandler;

};

#endif // __BUNDLE_H__
