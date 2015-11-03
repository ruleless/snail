#ifndef __LOGSYSTEM_H__
#define __LOGSYSTEM_H__

#include "common.h"
#include "thread/ThreadTask.h"
#include <semaphore.h>

#define MAX_LOGMSG_LEN 10

class ThreadPool;
class LogPipe;
class LogSystem : public TPTask
{
  public:
	enum ELevel
	{
		None,

		Trace,
		Warning,
		Error,

		MaxLevel,
	};
	enum ELogPipe
	{
		LogPipe_Stdout,

		LogPipe_Max,
	};

    LogSystem(ThreadPool *pThreadPool);
    virtual ~LogSystem();

	bool start();
	void stop();

	void log(ELevel level, std::string msg);

	virtual bool process();
	virtual TPTask::TPTaskState presentMainThread();
  private:
	std::string mLogMsg[MAX_LOGMSG_LEN];
	int mrpos;
	int mwpos;
	sem_t mEmpty;
	sem_t mStored;

	bool mStarted;
	ELevel mLogLevel;
	ThreadPool *mpThreadPool;

	LogPipe *mLogPipe[LogPipe_Max];
	LogPipe mPipeEnabled[LogPipe_Max];
};

class LogPipe
{
  public:
    LogPipe() {}
    virtual ~LogPipe() {}

	virtual void output(LogSystem::ELevel level, std::string msg) = 0;
};

#endif
