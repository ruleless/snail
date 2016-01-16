#include "DelayedChannels.h"
#include "Channel.h"
#include "Address.h"
#include "EventDispatcher.h"
#include "NetworkManager.h"

void DelayedChannels::initialize(EventDispatcher &dispatcher, NetworkManager *pNetworkInterface)
{
	mpNetworkManager = pNetworkInterface;
	dispatcher.addTask( this );
}

void DelayedChannels::finalise(EventDispatcher &dispatcher)
{
	dispatcher.cancelTask(this);
}

void DelayedChannels::add(Channel & channel)
{
	mChannelAddrs.insert(channel.addr());
}

void DelayedChannels::sendIfDelayed(Channel & channel)
{
	if (mChannelAddrs.erase(channel.addr()) > 0)
	{
		channel.send();
	}
}

bool DelayedChannels::process()
{
	ChannelAddrs::iterator iter = mChannelAddrs.begin();

	while (iter != mChannelAddrs.end())
	{
		Channel *pChannel = mpNetworkManager->findChannel((*iter));

		if (pChannel && (!pChannel->isCondemn() && !pChannel->isDestroyed()))
		{
			pChannel->send();
		}

		++iter;
	}

	mChannelAddrs.clear();
	return true;
}
