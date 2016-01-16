#ifndef __DELAYEDCHANNELS_H__
#define __DELAYEDCHANNELS_H__

#include "Tasks.h"
#include "SmartPointer.h"
#include "Singleton.h"

class Channel;
class Address;
class EventDispatcher;
class NetworkManager;

class DelayedChannels : public Task
{
public:
	void initialize(EventDispatcher &dispatcher, NetworkManager *pNetworkInterface);
	void finalise(EventDispatcher &dispatcher);

	void add(Channel &channel);
	void sendIfDelayed(Channel &channel);
private:
	virtual bool process();
private:
	typedef std::set<Address> ChannelAddrs;
	ChannelAddrs mChannelAddrs;

	NetworkManager *mpNetworkManager;
};

#endif // __DELAYEDCHANNELS_H__
