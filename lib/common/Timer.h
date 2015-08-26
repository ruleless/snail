#ifndef __TIMER_H__
#define __TIMER_H__

#include "common/common.h"
#include "common/timestamp.h"


class TimersBase;
class TimeBase;


// 创建定时器成功时，返回此对象，可用于取消定时器
class TimerHandle
{
public:
	explicit TimerHandle(TimeBase *pTime = NULL) : mpTime( pTime ) {}

	inline void cancel();

	void clearWithoutCancel()
	{
		mpTime = NULL; 
	}

	bool isSet() const
	{
		return mpTime != NULL; 
	}

	TimeBase* time() const
	{
		return mpTime;
	}

	friend bool operator==(TimerHandle h1, TimerHandle h2);
private:
	TimeBase *mpTime;
};

inline bool operator==(TimerHandle h1, TimerHandle h2)
{
	return h1.mpTime == h2.mpTime;
}


// 用到定时器的对象需继承此类
class TimerHandler
{
public:
	TimerHandler() : mNumTimesRegistered( 0 ) {}

	virtual ~TimerHandler()
	{
		Assert(mNumTimesRegistered == 0);
	};

	virtual void onTimeout(TimerHandle handle, void *pUser) = 0;
protected:
	virtual void onRelease( TimerHandle handle, void *pUser ) {}
private:
	friend class TimeBase;

	void incTimerRegisterCount() { ++mNumTimesRegistered; }
	void decTimerRegisterCount() { --mNumTimesRegistered; }

	void release( TimerHandle handle, void *pUser )
	{
		this->decTimerRegisterCount();
		this->onRelease(handle, pUser);
	}
private:
	int mNumTimesRegistered;
};

class TimeBase
{
public:
	TimeBase(TimersBase &owner, TimerHandler* pHandler, void* pUserData)
		:mOwner(owner), mpHandler(pHandler), mpUserData(pUserData), mState(Time_Pending)
	{
		pHandler->incTimerRegisterCount();
	}
	
	virtual ~TimeBase(){}

	inline void cancel();

	void* getUserData() const { return mpUserData; }

	bool isCancelled() const { return mState == Time_Cancelled; }
	bool isExecuting() const { return mState == Time_Executing; }
protected:
	enum ETimeState
	{
		Time_Pending,
		Time_Executing,
		Time_Cancelled,
	};

	TimersBase& mOwner;
	TimerHandler *mpHandler;
	void *mpUserData;
	ETimeState mState;
};


// 定时器管理类
class TimersBase
{
public:
	virtual void onCancel() = 0;
};

template<class TIME_STAMP> class TimersT : public TimersBase
{
public:
	typedef TIME_STAMP TimeStamp;

	TimersT();
	virtual ~TimersT();
	
	inline uint32 size() const { return mTimeQueue.size(); }
	inline bool empty() const { return mTimeQueue.empty(); }

	void clear(bool shouldCallCancel = true);
	
	TimerHandle	add(TimeStamp startTime, TimeStamp interval, TimerHandler* pHandler, void* pUser);
	int	process(TimeStamp now);

	bool legal(TimerHandle handle) const;
	TIME_STAMP nextExp(TimeStamp now) const;
	bool getTimerInfo(TimerHandle handle, TimeStamp& time, TimeStamp&	interval, void *&pUser) const;
private:
	TimersT(const TimersT &);
	TimersT & operator=(const TimersT &);

	virtual void onCancel();
	void purgeCancelledTimes();

	class Time : public TimeBase
	{
	public:
		Time(TimersBase &owner, TimeStamp startTime, TimeStamp interval, TimerHandler *pHandler, void *pUser);

		TIME_STAMP time() const { return mTime; }
		TIME_STAMP interval() const { return mInterval; }

		void triggerTimer();
	private:
		Time(const Time &);
		Time& operator=(const Time &);

		TimeStamp mTime;
		TimeStamp mInterval;
	};

	class Comparator
	{
	public:
		bool operator()(const Time* a, const Time* b)
		{
			return a->time() > b->time();
		}
	};
	
	class PriorityQueue
	{
	public:
		typedef std::vector<Time *> Container;

		typedef typename Container::value_type value_type;
		typedef typename Container::size_type size_type;

		bool empty() const { return mContainer.empty(); }
		size_type size() const { return mContainer.size(); }

		const value_type & top() const { return mContainer.front(); }

		void push( const value_type & x )
		{
			mContainer.push_back( x );
			std::push_heap(mContainer.begin(), mContainer.end(), Comparator());
		}

		void pop()
		{
			std::pop_heap(mContainer.begin(), mContainer.end(), Comparator());
			mContainer.pop_back();
		}

		Time* unsafePopBack()
		{
			Time *pTime = mContainer.back();
			mContainer.pop_back();
			return pTime;
		}

		Container & container() { return mContainer; }

		void make_heap()
		{
			std::make_heap(mContainer.begin(), mContainer.end(), Comparator());
		}
	private:
		Container mContainer;
	};
	
	PriorityQueue mTimeQueue;
	Time *mpProcessingNode;
	TimeStamp mLastProcessTime;
	int mNumCancelled;
};

typedef TimersT<uint32> Timers;
typedef TimersT<uint64> Timers64;

#include "Timer.inl"
#endif // __TIMER_H__
