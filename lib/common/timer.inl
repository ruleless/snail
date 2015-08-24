void TimerHandle::cancel()
{
	if (mpTime != NULL)
	{
		TimeBase* pTime = mpTime;
		mpTime = NULL;
		pTime->cancel();
	}
}


template<class TIME_STAMP>
TimersT<TIME_STAMP>::TimersT()
		:mTimeQueue()
		,mpProcessingNode(NULL)
		,mLastProcessTime(0)
		,mNumCancelled(0)
{
}

template<class TIME_STAMP>
TimersT<TIME_STAMP>::~TimersT()
{
	this->clear();
}

template <class TIME_STAMP>
void TimersT<TIME_STAMP>::clear(bool shouldCallCancel)
{
	int maxLoopCount = mTimeQueue.size();

	while (!mTimeQueue.empty())
	{
		Time * pTime = mTimeQueue.unsafePopBack();
		if (!pTime->isCancelled() && shouldCallCancel)
		{
			--mNumCancelled;
			pTime->cancel();

			if (--maxLoopCount == 0)
			{
				shouldCallCancel = false;
			}
		}
		else if (pTime->isCancelled())
		{
			--mNumCancelled;
		}

		delete pTime;
	}

	mNumCancelled = 0;
	mTimeQueue = PriorityQueue();
}

template <class TIME_STAMP> 
TimerHandle TimersT<TIME_STAMP>::add(TimeStamp startTime, TimeStamp interval, TimerHandler *pHandler, void *pUser)
{
	Time *pTime = new Time(*this, startTime, interval, pHandler, pUser);
	mTimeQueue.push(pTime);
	return TimerHandle(pTime);
}

template <class TIME_STAMP>
int TimersT<TIME_STAMP>::process(TimeStamp now)
{
	int numFired = 0;

	while ((!mTimeQueue.empty()) && (mTimeQueue.top()->time() <= now ||	mTimeQueue.top()->isCancelled()))
	{
		Time *pTime = mpProcessingNode = mTimeQueue.top();
		mTimeQueue.pop();

		if (!pTime->isCancelled())
		{
			++numFired;
			pTime->triggerTimer();
		}

		if (!pTime->isCancelled())
		{
			mTimeQueue.push( pTime );
		}
		else
		{
			delete pTime;

			Assert( mNumCancelled > 0 );
			--mNumCancelled;
		}
	}

	mpProcessingNode = NULL;
	mLastProcessTime = now;
	return numFired;
}

template <class TIME_STAMP>
bool TimersT<TIME_STAMP>::legal(TimerHandle handle) const
{
	typedef Time * const * TimeIter;
	Time *pTime = static_cast<Time *>(handle.time());

	if (pTime == NULL)
	{
		return false;
	}

	if (pTime == mpProcessingNode)
	{
		return true;
	}

	TimeIter begin = &mTimeQueue.top();
	TimeIter end = begin + mTimeQueue.size();

	for (TimeIter it = begin; it != end; ++it)
	{
		if (*it == pTime)
		{
			return true;
		}
	}

	return false;
}

template <class TIME_STAMP>
TIME_STAMP TimersT<TIME_STAMP>::nextExp(TimeStamp now) const
{
	if (mTimeQueue.empty() || now > mTimeQueue.top()->time())
	{
		return 0;
	}

	return mTimeQueue.top()->time() - now;
}

template <class TIME_STAMP>
bool TimersT<TIME_STAMP>::getTimerInfo(TimerHandle handle, TimeStamp &time, TimeStamp &interval, void *&pUser) const
{
	Time *pTime = static_cast< Time * >(handle.time());

	if (!pTime->isCancelled())
	{
		time = pTime->time();
		interval = pTime->interval();
		pUser = pTime->getUserData();

		return true;
	}

	return false;
}

template <class TIME_STAMP>
void TimersT<TIME_STAMP>::onCancel()
{
	++mNumCancelled;

	// If there are too many cancelled timers in the queue (more than half),
	// these are flushed from the queue immediately.
	if (mNumCancelled * 2 > int(mTimeQueue.size()))
	{
		this->purgeCancelledTimes();
	}
}

template <class TIME>
class IsNotCancelled
{
  public:
	bool operator()(const TIME *pTime)
	{
		return !pTime->isCancelled();
	}
};

template <class TIME_STAMP>
void TimersT<TIME_STAMP>::purgeCancelledTimes()
{
	typename PriorityQueue::Container & container = mTimeQueue.container();
	typename PriorityQueue::Container::iterator newEnd = std::partition(container.begin(), container.end(), IsNotCancelled<Time>());

	for (typename PriorityQueue::Container::iterator iter = newEnd; iter != container.end(); ++iter)
	{
		delete *iter;
	}

	const int numPurged = (container.end() - newEnd);
	mNumCancelled -= numPurged;
	Assert((mNumCancelled == 0) || (mNumCancelled == 1));
	
	container.erase(newEnd, container.end());
	mTimeQueue.make_heap();
}


TimeBase::TimeBase(TimersBase &owner, TimerHandler *pHandler, void *pUserData)
		:mOwner(owner), mpHandler(pHandler), mpUserData(pUserData), mState(Time_Pending)
{
	pHandler->incTimerRegisterCount();
}

void TimeBase::cancel()
{
	if (this->isCancelled())
	{
		return;
	}

	Assert((mState == Time_Pending) || (mState == Time_Executing));
	mState = Time_Cancelled;

	if (mpHandler)
	{
		mpHandler->release(TimerHandle(this), mpUserData);
		mpHandler = NULL;
	}

	mOwner.onCancel();
}

template <class TIME_STAMP>
TimersT<TIME_STAMP>::Time::Time(TimersBase &owner, TimeStamp startTime, TimeStamp interval, TimerHandler *_pHandler, void *_pUser) 
		:TimeBase(owner, _pHandler, _pUser), mTime(startTime), mInterval(interval)
{
}

template <class TIME_STAMP>
void TimersT<TIME_STAMP>::Time::triggerTimer()
{
	if (!this->isCancelled())
	{
		mState = Time_Executing;

		mpHandler->onTimeout( TimerHandle( this ), mpUserData );

		if ((mInterval == 0) && !this->isCancelled())
		{
			this->cancel();
		}
	}

	if (!this->isCancelled())
	{
		mTime += mInterval;
		mState = Time_Pending;
	}
}
