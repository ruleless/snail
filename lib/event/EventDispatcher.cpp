#include "EventDispatcher.h"
#include "network/EventPoller.h"

EventDispatcher::EventDispatcher()
		:breakProcessing_(DispatcherStatus_Running)
		,maxWait_(0.1)
		,numTimerCalls_(0)
		,accSpareTime_(0)
		,oldSpareTime_(0)
		,totSpareTime_(0)
		,lastStatisticsGathered_(0)   
{
	pTasks_ = new Tasks();
	pTimers_ = new Timers64();
	pPoller_ = EventPoller::create();
}

EventDispatcher::~EventDispatcher()
{
	SafeDelete(pTasks_);
	SafeDelete(pPoller_);
	
	if (!pTimers_->empty())
	{
		INFO_MSG(fmt::format("EventDispatcher()::~EventDispatcher: Num timers = {}\n",
							 pTimers_->size()));
	}

	pTimers_->clear(false);
	SafeDelete(pTimers_);
}

bool EventDispatcher::registerReadFileDescriptor(int fd, InputNotificationHandler *handler)
{
	return pPoller_->registerForRead(fd, handler);
}

bool EventDispatcher::registerWriteFileDescriptor(int fd, OutputNotificationHandler *handler)
{
	return pPoller_->registerForWrite(fd, handler);
}

bool EventDispatcher::deregisterReadFileDescriptor(int fd)
{
	return pPoller_->deregisterForRead(fd);
}

bool EventDispatcher::deregisterWriteFileDescriptor(int fd)
{
	return pPoller_->deregisterForWrite(fd);
}

uint64 EventDispatcher::getSpareTime() const
{
	return pPoller_->spareTime();
}

void EventDispatcher::clearSpareTime()
{
	accSpareTime_ += pPoller_->spareTime();
	pPoller_->clearSpareTime();
}

double EventDispatcher::calculateWait() const
{
	double maxWait = maxWait_;

	if (!pTimers_->empty())
	{
		maxWait = std::min(maxWait, pTimers_->nextExp(timestamp()) / stampsPerSecondD());
	}

	return maxWait;
}

TimerHandle EventDispatcher::addTimerCommon(int64 microseconds, TimerHandler *handler, void *arg, bool recurrent)
{
	Assert(handler);

	if (microseconds <= 0)
		return TimerHandle();

	uint64 interval = int64((((double)microseconds)/1000000.0) * stampsPerSecondD());

	TimerHandle handle = pTimers_->add(timestamp() + interval,
									   recurrent ? interval : 0,
									   handler, arg);
	
	return handle;
}

void EventDispatcher::addTask(Task *pTask)
{
	pTasks_->add(pTask);
}

bool EventDispatcher::cancelTask(Task * pTask)
{
	return pTasks_->cancel(pTask);
}

void EventDispatcher::processUntilBreak()
{
	if(breakProcessing_ != DispatcherStatus_BreakProcessing)
		breakProcessing_ = DispatcherStatus_Running;

	while(breakProcessing_ != DispatcherStatus_BreakProcessing)
	{
		this->processOnce(true);
	}
}

int EventDispatcher::processOnce(bool shouldIdle)
{
	if(breakProcessing_ != DispatcherStatus_BreakProcessing)
		breakProcessing_ = DispatcherStatus_Running;

	this->processTasks();

	if(breakProcessing_ != DispatcherStatus_BreakProcessing)
	{
		this->processTimers();
	}

	this->processStats();

	if(breakProcessing_ != DispatcherStatus_BreakProcessing)
	{
		return this->processNetwork(shouldIdle);
	}

	return 0;
}

int EventDispatcher::processNetwork(bool shouldIdle)
{
	double maxWait = shouldIdle ? this->calculateWait() : 0.0;
	return pPoller_->processPendingEvents(maxWait);
}

void EventDispatcher::processTasks()
{
	pTasks_->process();
}

void EventDispatcher::processTimers()
{
	numTimerCalls_ += pTimers_->process(timestamp());
}

void EventDispatcher::processStats()
{
	if (timestamp() - lastStatisticsGathered_ >= stampsPerSecond())
	{
		oldSpareTime_ = totSpareTime_;
		totSpareTime_ = accSpareTime_ + pPoller_->spareTime();

		lastStatisticsGathered_ = timestamp();
	}
}
