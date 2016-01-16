#include "common/common.h"
#include "Thread.h"

static void* threadFunc(void*);

Thread::Thread()
		:mTID(0)
		,mStatus(S_Ready)
{
}

Thread::~Thread()
{
}

void Thread::start() {
	if (mStatus == S_Ready)
	{
		pthread_create(&mTID, NULL, threadFunc, this);
	}
}

void Thread::wait(void *status)
{
	if (mStatus != S_Ready)
	{
		pthread_join(mTID, &status);
		mStatus = S_Ready;
	}
}

Thread::Status Thread::getStatus() const
{
	return mStatus;
}

void Thread::_setStatus(Status s)
{
	mStatus = s;
}

pthread_t Thread::getThreadID() const
{
	return mTID;
}

void Thread::run()
{
}

static void* threadFunc(void* arg)
{
	Thread *pThread = (Thread *)arg;

	pThread->_setStatus(Thread::S_Running);
	pThread->run();
	pThread->_setStatus(Thread::S_Exiting);
	
	return NULL;
}
