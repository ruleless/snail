#include "NetworkStats.h"
#include "helper/watcher.h"
#include "network/MessageHandler.h"

SINGLETON_INIT(NetworkStats);

NetworkStats gNetworkStats;

NetworkStats::NetworkStats()
:mStats()
,mHandlers()
{
}

NetworkStats::~NetworkStats()
{
}

void NetworkStats::trackMessage(EOptType op, const MessageHandler& msgHandler, uint32 size)
{
	MessageHandler* pMsgHandler = const_cast<MessageHandler *>(&msgHandler);

	if(op == Opt_Send)
	{
		pMsgHandler->send_size += size;
		pMsgHandler->send_count++;
	}
	else
	{
		pMsgHandler->recv_size += size;
		pMsgHandler->recv_count++;
	}

	std::vector<NetworkStatsHandler *>::iterator iter = mHandlers.begin();
	for(; iter != mHandlers.end(); ++iter)
	{
		if(op == Opt_Send)
			(*iter)->onSendMessage(msgHandler, size);
		else
			(*iter)->onRecvMessage(msgHandler, size);
	}
}

void NetworkStats::addHandler(NetworkStatsHandler* pHandler)
{
	mHandlers.push_back(pHandler);
}

void NetworkStats::removeHandler(NetworkStatsHandler* pHandler)
{
	std::vector<NetworkStatsHandler*>::iterator iter = mHandlers.begin();
	for(; iter != mHandlers.end(); ++iter)
	{
		if((*iter) == pHandler)
		{
			mHandlers.erase(iter);
			break;
		}
	}
}