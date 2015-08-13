#ifndef __EVENTDISPATCHER_H__
#define __EVENTDISPATCHER_H__

#include <map>
#include "common/tasks.h"
#include "common/timer.h"
#include "network/interfaces.h"
#include "network/common.h"

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

	INLINE bool hasBreakProcessing() const
	{
		return DispatcherStatus_BreakProcessing == breakProcessing_; 
	}
	INLINE bool waitingBreakProcessing() const
	{
		return DispatcherStatus_WaitingBreakProcessing == breakProcessing_; 
	}
	INLINE void setWaitBreakProcessing()
	{
		breakProcessing_ = DispatcherStatus_WaitingBreakProcessing;
	}
	INLINE void breakProcessing(bool breakState = true)
	{
		if(breakState)
			breakProcessing_ = DispatcherStatus_BreakProcessing;
		else
			breakProcessing_ = DispatcherStatus_Running;
	}

	INLINE double maxWait() const
	{
		return maxWait_;
	}

	INLINE void maxWait(double seconds)
	{
		maxWait_ = seconds;
	}

	INLINE TimerHandle addTimer(int64 microseconds, TimerHandler * handler, void* arg = NULL)
	{
		return this->addTimerCommon(microseconds, handler, arg, true);
	}

	INLINE EventPoller* pPoller()
	{
		return pPoller_;
	}

	int processNetwork(bool shouldIdle);
  private:
	TimerHandle addTimerCommon(int64 microseconds, TimerHandler *handler, void *arg, bool recurrent);

	void processTasks();
	void processTimers();
	void processStats();
	
	double calculateWait() const;	
  protected:
	int8 breakProcessing_;

	double maxWait_;
	uint32 numTimerCalls_;
	
	TimeStamp accSpareTime_;
	TimeStamp oldSpareTime_;
	TimeStamp totSpareTime_;
	TimeStamp lastStatisticsGathered_;
	
	Tasks *pTasks_;
	Timers64 *pTimers_;
	EventPoller * pPoller_;
};

#endif
