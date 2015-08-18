#include "EventDispatcher.h"
#include "network/EventPoller.h"

EventDispatcher::EventDispatcher()
		:mBreakProcessing(DispatcherStatus_Running)
		,mMaxWait(0.1)
		,mNumTimerCalls(0)
		,mAccSpareTime(0)
		,mOldSpareTime(0)
		,mTotSpareTime(0)
		,mLastStatisticsGathered(0)   
{
	mpTasks = new Tasks();
	mpTimers = new Timers64();
	mpPoller = EventPoller::create();
}

EventDispatcher::~EventDispatcher()
{
	SafeDelete(mpTasks);
	SafeDelete(mpPoller);
	
	if (!mpTimers->empty())
	{
// 		INFO_MSG(fmt::format("EventDispatcher()::~EventDispatcher: Num timers = {}\n",
// 							 mpTimers->size()));
	}

	mpTimers->clear(false);
	SafeDelete(mpTimers);
}

bool EventDispatcher::registerReadFileDescriptor(int fd, InputNotificationHandler *handler)
{
	return mpPoller->registerForRead(fd, handler);
}

bool EventDispatcher::registerWriteFileDescriptor(int fd, OutputNotificationHandler *handler)
{
	return mpPoller->registerForWrite(fd, handler);
}

bool EventDispatcher::deregisterReadFileDescriptor(int fd)
{
	return mpPoller->deregisterForRead(fd);
}

bool EventDispatcher::deregisterWriteFileDescriptor(int fd)
{
	return mpPoller->deregisterForWrite(fd);
}

uint64 EventDispatcher::getSpareTime() const
{
	return mpPoller->spareTime();
}

void EventDispatcher::clearSpareTime()
{
	mAccSpareTime += mpPoller->spareTime();
	mpPoller->clearSpareTime();
}

double EventDispatcher::calculateWait() const
{
	double maxWait = mMaxWait;

	if (!mpTimers->empty())
	{
		maxWait = min(maxWait, mpTimers->nextExp(timestamp()) / stampsPerSecondD());
	}

	return maxWait;
}

TimerHandle EventDispatcher::addTimerCommon(int64 microseconds, TimerHandler *handler, void *arg, bool recurrent)
{
	Assert(handler);

	if (microseconds <= 0)
		return TimerHandle();

	uint64 interval = int64((((double)microseconds)/1000000.0) * stampsPerSecondD());

	TimerHandle handle = mpTimers->add(timestamp() + interval, recurrent ? interval : 0, handler, arg);
	
	return handle;
}

void EventDispatcher::addTask(Task *pTask)
{
	mpTasks->add(pTask);
}

bool EventDispatcher::cancelTask(Task * pTask)
{
	return mpTasks->cancel(pTask);
}

void EventDispatcher::processUntilBreak()
{
	if(mBreakProcessing != DispatcherStatus_BreakProcessing)
		mBreakProcessing = DispatcherStatus_Running;

	while(mBreakProcessing != DispatcherStatus_BreakProcessing)
	{
		this->processOnce(true);
	}
}

int EventDispatcher::processOnce(bool shouldIdle)
{
	if(mBreakProcessing != DispatcherStatus_BreakProcessing)
		mBreakProcessing = DispatcherStatus_Running;

	this->processTasks();

	if(mBreakProcessing != DispatcherStatus_BreakProcessing)
	{
		this->processTimers();
	}

	this->processStats();

	if(mBreakProcessing != DispatcherStatus_BreakProcessing)
	{
		return this->processNetwork(shouldIdle);
	}

	return 0;
}

int EventDispatcher::processNetwork(bool shouldIdle)
{
	double maxWait = shouldIdle ? this->calculateWait() : 0.0;
	return mpPoller->processPendingEvents(maxWait);
}

void EventDispatcher::processTasks()
{
	mpTasks->process();
}

void EventDispatcher::processTimers()
{
	mNumTimerCalls += mpTimers->process(timestamp());
}

void EventDispatcher::processStats()
{
	if (timestamp() - mLastStatisticsGathered >= stampsPerSecond())
	{
		mOldSpareTime = mTotSpareTime;
		mTotSpareTime = mAccSpareTime + mpPoller->spareTime();

		mLastStatisticsGathered = timestamp();
	}
}
