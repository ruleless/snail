#ifndef __EVENTDISPATCHER_H__
#define __EVENTDISPATCHER_H__

#include <map>
#include "common/Tasks.h"
#include "common/Timer.h"
#include "network/Network.h"
#include "network/NetworkDef.h"

class EventPoller;

class EventDispatcher
{
  public:
	enum EDispatcherStatus
	{
		DispatcherStatus_Running = 0,
		DispatcherStatus_WaitingBreakProcessing,
		DispatcherStatus_BreakProcessing,
	};

	EventDispatcher();
	virtual ~EventDispatcher();
	
	void processUntilBreak();
	int processOnce(bool shouldIdle = false);

	bool registerReadFileDescriptor(int fd, InputNotificationHandler *handler);
	bool deregisterReadFileDescriptor(int fd);
	bool registerWriteFileDescriptor(int fd, OutputNotificationHandler *handler);
	bool deregisterWriteFileDescriptor(int fd);	

	uint64 getSpareTime() const;
	void clearSpareTime(); 

	void addTask(Task *pTask);
	bool cancelTask(Task *pTask);

	inline TimerHandle addTimer(int64 microseconds, TimerHandler *handler, void *arg = NULL)
	{
		return this->addTimerCommon(microseconds, handler, arg, true);
	}

	inline bool hasBreakProcessing() const
	{
		return DispatcherStatus_BreakProcessing == mBreakProcessing; 
	}
	inline bool waitingBreakProcessing() const
	{
		return DispatcherStatus_WaitingBreakProcessing == mBreakProcessing; 
	}
	inline void setWaitBreakProcessing()
	{
		mBreakProcessing = DispatcherStatus_WaitingBreakProcessing;
	}
	inline void breakProcessing(bool breakState = true)
	{
		if(breakState)
			mBreakProcessing = DispatcherStatus_BreakProcessing;
		else
			mBreakProcessing = DispatcherStatus_Running;
	}

	inline double maxWait() const
	{
		return mMaxWait;
	}

	inline void maxWait(double seconds)
	{
		mMaxWait = seconds;
	}	

	inline EventPoller* pPoller()
	{
		return mpPoller;
	}

	int processNetwork(bool shouldIdle);
  private:
	TimerHandle addTimerCommon(int64 microseconds, TimerHandler *handler, void *arg, bool recurrent);

	void processTasks();
	void processTimers();
	void processStats();
	
	double calculateWait() const;	
  protected:
	int8 mBreakProcessing;

	double mMaxWait;
	uint32 mNumTimerCalls;
	
	TimeStamp mAccSpareTime;
	TimeStamp mOldSpareTime;
	TimeStamp mTotSpareTime;
	TimeStamp mLastStatisticsGathered;
	
	Tasks *mpTasks;
	Timers64 *mpTimers;
	EventPoller *mpPoller;
};

#endif
