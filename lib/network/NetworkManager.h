#ifndef __NETWORKMANAGER_H__
#define __NETWORKMANAGER_H__

#include "common/common.h"
#include "common/Timer.h"
#include "common/MemoryStream.h"
#include "network/NetworkDef.h"
#include "network/EndPoint.h"

class Address;
class Bundle;
class Channel;
class ChannelTimeOutHandler;
class ChannelDeregisterHandler;
class DelayedChannels;
class Listener;
class Packet;
class EventDispatcher;
class MessageHandlers;

class NetworkManager : public TimerHandler
{
  public:
	typedef std::map<Address, Channel *> ChannelMap;
	
	NetworkManager(EventDispatcher *pDispatcher,
				   int32 extlisteningPort_min = -1, 
				   int32 extlisteningPort_max = -1,
				   const char *extlisteningInterface = "",
				   uint32 extrbuffer = 0, 
				   uint32 extwbuffer = 0, 
				   int32 intlisteningPort = 0,
				   const char *intlisteningInterface = "",
				   uint32 intrbuffer = 0,
				   uint32 intwbuffer = 0);

	~NetworkManager();

	const char* c_str() const { return mExtEndpoint.c_str(); }

	const Address& extaddr() const { return mExtEndpoint.addr(); }
	const Address& intaddr() const { return mIntEndpoint.addr(); }

	int32 numExtChannels() const { return mNumExtChannels; }

	bool recreateListeningSocket(const char *pEndPointName,
								 uint16 minListeningPort, 
								 uint16 maxListeningPort, 
								 const char *listeningInterface, 
								 EndPoint *pEndPoint,
								 Listener *pListener,
								 uint32 rbuffer = 0, 
								 uint32 wbuffer = 0);

	const ChannelMap& channels(void) { return mChannels; }
	void processChannels(MessageHandlers* pMsgHandlers);
	bool registerChannel(Channel* pChannel);
	bool deregisterChannel(Channel* pChannel);
	bool deregisterAllChannels();
	Channel* findChannel(const Address & addr);
	Channel* findChannel(int fd);
	void onChannelTimeOut(Channel * pChannel);

	void delayedSend(Channel & channel);
	void sendIfDelayed(Channel & channel);

	ChannelTimeOutHandler* getChannelTimeOutHandler() const { return mpChannelTimeOutHandler; }
	void setChannelTimeOutHandler(ChannelTimeOutHandler * pHandler) { mpChannelTimeOutHandler = pHandler; }
		
	ChannelDeregisterHandler* getChannelDeregisterHandler() const { return mpChannelDeregisterHandler; }
	void setChannelDeregisterHandler(ChannelDeregisterHandler * pHandler)	{ mpChannelDeregisterHandler = pHandler; }

	EventDispatcher& dispatcher() { return *mpDispatcher; }

	EndPoint& extEndpoint() { return mExtEndpoint; }
	EndPoint& intEndpoint() { return mIntEndpoint; }
	
	bool isExternal() const { return mIsExternal; }

	void* pExtensionData() const     { return mpExtensionData; }
	void pExtensionData(void * pData) { mpExtensionData = pData; }
	
	bool good() const{ return (!isExternal() || mExtEndpoint.good()) && (mIntEndpoint.good()); }
  private:
	virtual void onTimeout(TimerHandle handle, void * arg);

	void closeSocket();
  private:
	Listener *mpExtListenerReceiver;
	EndPoint mExtEndpoint;
	
	Listener *mpIntListenerReceiver;
	EndPoint mIntEndpoint;

	ChannelMap mChannels;

	void *mpExtensionData;
	const bool mIsExternal;
	int32 mNumExtChannels;

	DelayedChannels *mpDelayedChannels;
	
	ChannelTimeOutHandler *mpChannelTimeOutHandler;
	ChannelDeregisterHandler *mpChannelDeregisterHandler;

	EventDispatcher *mpDispatcher;
};

#endif // __NETWORKMANAGER_H__
