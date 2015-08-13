#ifndef __PACKET_H__
#define __PACKET_H__

#include "common/memorystream.h"
#include "common/common.h"
#include "common/ObjectPool.h"
#include "common/smartpointer.h"	
#include "network/common.h"

class EndPoint;
class Address;
class Bundle;

class Packet : public MemoryStream, public RefCountable
{
public:
	Packet(MessageID msgID = 0, bool isTCPPacket = true, size_t res = 200)
		:MemoryStream(res)
		,mMsgID(msgID)
		,mIsTCPPacket(isTCPPacket)
		,mbEncrypted(false)
		,mpBundle(NULL)
		,sentSize(0)
	{
	}
	
	virtual ~Packet(void)
	{
	}
	
	virtual void onReclaimObject()
	{
		MemoryStream::onReclaimObject();
		resetPacket();
	}
	
	virtual size_t getPoolObjectBytes()
	{
		size_t bytes = sizeof(mMsgID) + sizeof(mIsTCPPacket) + sizeof(mbEncrypted) + sizeof(mpBundle) + sizeof(sentSize);

		return MemoryStream::getPoolObjectBytes() + bytes;
	}

	Bundle* getBundle() const
	{
		return mpBundle;
	}
	void setBundle(Bundle* v)
	{
		mpBundle = v;
	}

	virtual int recvFromEndPoint(EndPoint &ep, Address *pAddr = NULL) = 0;
    virtual bool empty() const
	{
		return length() == 0; 
	}

	void resetPacket(void)
	{
		wpos(0);
		rpos(0);
		mbEncrypted = false;
		sentSize = 0;
		mMsgID = 0;
		mpBundle = NULL;
		// memset(data(), 0, size());
	};
	
	INLINE void setMessageID(MessageID msgID)
	{ 
		mMsgID = msgID; 
	}

	INLINE MessageID getMessageID() const
	{
		return mMsgID; 
	}

	void isTCPPacket(bool v)
	{
		mIsTCPPacket = v; 
	}
	bool isTCPPacket() const
	{
		return mIsTCPPacket; 
	}

	bool encrypted() const
	{
		return mbEncrypted; 
	}

	void encrypted(bool v)
	{
		mbEncrypted = v; 
	}
public:
	uint32 sentSize;
protected:
	MessageID mMsgID;
	bool mIsTCPPacket;
	bool mbEncrypted;

	Bundle *mpBundle;
};

#endif // __PACKET_H__
