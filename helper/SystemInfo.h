#ifndef __SYSTEMINFO_H__
#define __SYSTEMINFO_H__

#include "common/common.h"
#include "common/singleton.h"

class SystemInfo : public Singleton<SystemInfo>
{
public:
	SystemInfo();
	~SystemInfo();

	struct PROCESS_INFOS
	{
		float cpu;
		uint64 memused;
		bool error;
	};

	struct MEM_INFOS
	{
		uint64 total;
		uint64 free;
		uint64 used;
	};

	SystemInfo::MEM_INFOS getMemInfos();
	SystemInfo::PROCESS_INFOS getProcessInfo(uint32 pid = 0);

	float getCPUPer();
	float getCPUPerByPID(uint32 pid = 0);
	uint64 getMemUsedByPID(uint32 pid = 0);
	bool hasProcess(uint32 pid);

	uint32 countCPU();

	uint64 totalmem();

	void update();
	
	void clear();
private:
	bool _autocreate();
private:
	uint64 mTotalMem;
};

extern SystemInfo gSystemInfo;

#endif // __SYSTEMINFO_H__