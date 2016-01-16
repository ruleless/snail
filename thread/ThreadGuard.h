#ifndef __THREADGUARD_H__
#define __THREADGUARD_H__
	
#include "thread/ThreadMutex.h"
#include <assert.h>

class ThreadGuard
{
public:
	explicit ThreadGuard(ThreadMutex* mutexPtr) : mpMutex(mutexPtr)
	{
		mpMutex->lockMutex();
	}

	virtual ~ThreadGuard(void) 
	{ 
		mpMutex->unlockMutex();
	}	
	
protected:
	ThreadMutex* mpMutex;
};

#endif // __THREADGUARD_H__
