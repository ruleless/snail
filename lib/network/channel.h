#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include "common/common.h"
#include "common/Timer.h"
#include "common/SmartPointer.h"
#include "common/timestamp.h"
#include "common/ObjectPool.h"
#include "network/address.h"
#include "network/EventDispatcher.h"
#include "network/EndPoint.h"
#include "network/Packet.h"
#include "network/NetworkDef.h"
#include "network/Bundle.h"
#include "network/Network.h"
#include "network/PacketFilter.h"

class Bundle;
class NetworkManager;
class MessageHandlers;
class PacketReader;
class PacketSender;

class Channel : public TimerHandler, public PoolObject
{
public:
	static ObjectPool<Channel>& ObjPool();
	static void destroyObjPool();
	virtual void onReclaimObject();
	virtual size_t getPoolObjectBytes();

	enum ETraits
	{
		Internal = 0,
		External,
	};
	
	enum EChannelTypes
	{
		Chanel_Normal = 0,
		Chanel_Web,
	};

	Channel();

	Channel(NetworkManager &networkInterface, 
		const EndPoint *pEndPoint,
		ETraits traits,
		ProtocolType pt = Protocol_TCP, 
		PacketFilterPtr pFilter = NULL, 
		ChannelID id = CHANNEL_ID_NULL);

	virtual ~Channel();

	const char* c_str() const;
	ChannelID id() const { return mID; }

	bool initialize(NetworkManager & networkInterface, 
		const EndPoint * pEndPoint, 
		ETraits traits, 
		ProtocolType pt = Protocol_TCP, 
		PacketFilterPtr pFilter = NULL, 
		ChannelID id = CHANNEL_ID_NULL);

	bool finalise();

	void destroy();
	bool isDestroyed() const { return (mFlags & Flag_Destroyed) > 0; }

	void clearState( bool warnOnDiscard = false );
	
	void startInactivityDetection(float inactivityPeriod, float checkPeriod = 1.f);
	void stopInactivityDetection();

	PacketFilterPtr getFilter() const { return mpFilter; }
	void setFilter(PacketFilterPtr pFilter) { mpFilter = pFilter; }

	NetworkManager& getNetworkManager() { return *mpNetworkManager; }

	const Address& addr() const { return mpEndPoint->addr(); }
	EndPoint* pEndPoint() const { return mpEndPoint; }
	void pEndPoint(const EndPoint* pEndPoint);

	typedef std::vector<Bundle *> Bundles;
	const Bundles& bundles() const { return mBundles; }
	Bundles& bundles()             { return mBundles; }
	int32 bundlesLength();
	void clearBundle();

	bool sending() const { return (mFlags & Flag_Sending) > 0;}
	void send(Bundle * pBundle = NULL);
	void stopSend();
	void delayedSend();
	void onSendCompleted();
	void onPacketSent(int bytes, bool sentCompleted);

	void onPacketReceived(int bytes);
	void addReceiveWindow(Packet* pPacket);

	void processPackets(MessageHandlers* pMsgHandlers);

	PacketReader* pPacketReader() const { return mpPacketReader; }
	PacketSender* pPacketSender() const { return mpPacketSender; }
	void pPacketSender(PacketSender* pPacketSender) { mpPacketSender = pPacketSender; }
	PacketReceiver* pPacketReceiver() const { return mpPacketReceiver; }

	ETraits traits() const   { return mTraits; }
	bool isExternal() const { return mTraits == External; }
	bool isInternal() const { return mTraits == Internal; }
	
	uint32 numPacketsSent() const     { return mNumPacketsSent; }
	uint32 numPacketsReceived() const { return mNumPacketsReceived; }
	uint32 numBytesSent() const       { return mNumBytesSent; }
	uint32 numBytesReceived() const   { return mNumBytesReceived; }
		
	uint64 lastReceivedTime() const { return mLastReceivedTime; }
	void updateLastReceivedTime()   { mLastReceivedTime = timestamp(); }

	bool isCondemn() const    { return (mFlags & Flag_Condemn) > 0; }
	bool hasHandshake() const { return (mFlags & Flag_HandShake) > 0; }
	void condemn();
	virtual void handshake();

	int32 proxyID() const { return proxyID_; }
	void proxyID(int32 pid){ proxyID_ = pid; }

	const std::string& extra() const { return mStrExtra; }
	void extra(const std::string& s){ mStrExtra = s; }

	COMPONENT_ID componentID() const{ return mComponentID; }
	void componentID(COMPONENT_ID cid){ mComponentID = cid; }

	bool waitSend();

	virtual void onTimeout(TimerHandle, void * pUser);
private:
	enum EFlags
	{
		Flag_Sending	= 0x00000001,	// 发送信息中
		Flag_Destroyed	= 0x00000002,	// 通道已经销毁
		Flag_HandShake	= 0x00000004,	// 已经握手过
		Flag_Condemn	= 0x00000008,	// 该频道已经变得不合法
	};

	enum ETimeOutType
	{
		Timeout_InactivityCheck,
	};

	typedef std::vector<Packet *> BufferedReceives;

	BufferedReceives mBufferedReceives;

	NetworkManager *mpNetworkManager;
	ChannelID mID;
	ETraits mTraits;
	ProtocolType mProtocolType;

	TimerHandle	mInactivityTimerHandle;
	uint64 mInactivityExceptionPeriod;
	uint64 mLastReceivedTime;

	Bundles	mBundles;

	EndPoint *mpEndPoint;
	PacketFilterPtr	mpFilter;
	PacketReader *mpPacketReader;
	PacketReceiver *mpPacketReceiver;
	PacketSender *mpPacketSender;

	// 统计数据
	uint32 mNumPacketsSent;
	uint32 mNumPacketsReceived;
	uint32 mNumBytesSent;
	uint32 mNumBytesReceived;
	uint32 mLastTickBytesReceived;
	uint32 mLastTickBytesSent;

	// 如果是外部通道且代理了一个前端则会绑定前端代理ID
	int32 proxyID_;
	EChannelTypes mChannelType;
	COMPONENT_ID mComponentID;
	uint32 mFlags;

	// 扩展用
	std::string	mStrExtra;
};

#endif // __CHANNEL_H__
