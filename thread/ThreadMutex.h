#ifndef __THREADMUTEX_H__
#define __THREADMUTEX_H__
	
#include "common/common.h"

class ThreadMutex 
{
public:
	ThreadMutex(void)
	{
		THREAD_MUTEX_INIT(mMutex);
	}

	virtual ~ThreadMutex(void) 
	{ 
		THREAD_MUTEX_DELETE(mMutex);
	}	
	
	void lockMutex(void)
	{
		THREAD_MUTEX_LOCK(mMutex);
	}

	void unlockMutex(void)
	{
		THREAD_MUTEX_UNLOCK(mMutex);
	}
protected:
	THREAD_MUTEX mMutex;
};

#endif // __THREADMUTEX_H__
