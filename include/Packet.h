#ifndef __PACKET_H__
#define __PACKET_H__

#include "MemoryStream.h"
#include "common.h"
#include "ObjectPool.h"
#include "SmartPointer.h"	
#include "NetworkDef.h"

class EndPoint;
class Address;
class Bundle;

class Packet : public MemoryStream, public RefCountable
{
  public:
	Packet(MessageID msgID = 0, bool isTCPPacket = true, size_t res = 200)
			:MemoryStream(res)
			,sentSize(0)
			,mMsgID(msgID)
			,mIsTCPPacket(isTCPPacket)
			,mbEncrypted(false)
			,mpBundle(NULL)			
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
	
	inline void setMessageID(MessageID msgID)
	{ 
		mMsgID = msgID; 
	}

	inline MessageID getMessageID() const
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
