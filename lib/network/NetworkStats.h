#ifndef __NETWORKSTATS_H__
#define __NETWORKSTATS_H__

#include "common/common.h"
#include "common/Singleton.h"
#include "network/Network.h"

class MessageHandler;

// 记录network流量等信息
class NetworkStats : public Singleton<NetworkStats>
{
public:
	enum EOptType
	{
		Opt_Send,
		Opt_Recv,
	};

	struct Stats
	{
		Stats()
		{
			name = "";
			send_count = 0;
			send_size = 0;
			recv_size = 0;
			recv_count = 0;
		}

		std::string name;
		uint32 send_size;
		uint32 send_count;
		uint32 recv_size;
		uint32 recv_count;
	};

	typedef UnorderedMap<std::string, Stats> STATS;

	NetworkStats();
	~NetworkStats();

	void trackMessage(EOptType op, const MessageHandler& msgHandler, uint32 size);

	NetworkStats::STATS& stats() { return mStats; }

	void addHandler(NetworkStatsHandler* pHandler);
	void removeHandler(NetworkStatsHandler* pHandler);
private:
	STATS mStats;

	std::vector<NetworkStatsHandler *> mHandlers;
};

extern NetworkStats gNetworkStats;

#endif // __NETWORKSTATS_H__
