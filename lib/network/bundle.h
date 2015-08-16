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
	void finiMessage(bool bReadyToSend = true);

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
    Bundle &operator<<(uint8 value);
    Bundle &operator<<(uint16 value);
    Bundle &operator<<(uint32 value);
    Bundle &operator<<(uint64 value);
    Bundle &operator<<(int8 value);
    Bundle &operator<<(int16 value);
    Bundle &operator<<(int32 value);
    Bundle &operator<<(int64 value);
    Bundle &operator<<(float value);
    Bundle &operator<<(double value);
    Bundle &operator<<(bool value);
    Bundle &operator<<(const std::string &value);	
    Bundle &operator<<(const char *str);    

	Bundle &append(Bundle* pBundle)
	Bundle &append(Bundle& bundle);
	Bundle &append(MemoryStream* s);
	Bundle &append(MemoryStream& s);
	Bundle &append(const uint8 *str, int n);
	Bundle &append(const char *str, int n);

	Bundle &appendBlob(const std::string& str);
	Bundle &appendBlob(const char* str, ArraySize n);
	Bundle &appendBlob(const uint8 *str, ArraySize n);

	Bundle &assign(const char *str, int n);

    Bundle &operator>>(bool &value);
    Bundle &operator>>(uint8 &value);
    Bundle &operator>>(uint16 &value);
    Bundle &operator>>(uint32 &value);
    Bundle &operator>>(uint64 &value);
    Bundle &operator>>(int8 &value);
    Bundle &operator>>(int16 &value);
    Bundle &operator>>(int32 &value);
    Bundle &operator>>(int64 &value);
    Bundle &operator>>(float &value);
    Bundle &operator>>(double &value);
    Bundle &operator>>(std::string& value);

	ArraySize readBlob(std::string& datas);
private:
	template<typename T> Bundle &outputValue(T &v);
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

#include "Bundle.inl"

#endif // __BUNDLE_H__
