#include "ThreadPool.h"

SINGLETON_INIT(ThreadPool);

int ThreadPool::timeout = 300;

THREAD_ID TPThread::createThread(void)
{
#if PLATFORM == PLATFORM_WIN32
	mTid = (THREAD_ID)_beginthreadex(NULL, 0, &TPThread::threadFunc, (void*)this, NULL, 0);
#else	
	if(pthread_create(&mTid, NULL, TPThread::threadFunc, (void*)this)!= 0)
	{
		// ERROR_MSG("createThread error!");
	}
#endif
	return mTid;
}

bool TPThread::join(void)
{
#if PLATFORM == PLATFORM_WIN32
	int i = 0;

	while(true)
	{
		++i;
		DWORD dw = WaitForSingleObject(id(), 3000);  

		switch (dw)
		{
		case WAIT_OBJECT_0:
			return true;
		case WAIT_TIMEOUT:
			if(i > 20)
			{
				// ERROR_MSG(fmt::format("TPThread::join: can't join thread({0:p})\n", (void*)this));
				return false;
			}
			else
			{
				// WARNING_MSG(fmt::format("TPThread::join: waiting for thread({0:p}), try={1}\n", (void*)this, i));
			}
			break;
		case WAIT_FAILED:
		default:
			// ERROR_MSG(fmt::format("TPThread::join: can't join thread({0:p})\n", (void*)this));
			return false;
		};
	}
#else
	void* status;
	if(pthread_join(id(), &status))
	{
		// ERROR_MSG(fmt::format("TPThread::join: can't join thread({0:p})\n", (void*)this));
		return false;
	}
#endif

	return true;
}

void TPThread::onTaskCompleted(void)
{
	mpThreadPool->addFiniTask(mpCurrTask);
	mpCurrTask = NULL;
	mpThreadPool->addFreeThread(this);
}

bool TPThread::onWaitCondSignal(void)
{
#if PLATFORM == PLATFORM_WIN32
	if(mThreadWaitSecond <= 0)
	{
		mState = ThreadState_Sleep;
		WaitForSingleObject(mCond, INFINITE); 
		ResetEvent(mCond);
	}
	else
	{
		mState = ThreadState_Sleep;
		DWORD ret = WaitForSingleObject(mCond, mThreadWaitSecond * 1000);
		ResetEvent(mCond);

		// 如果是因为超时了， 说明这个线程很久没有被用到， 我们应该注销这个线程。
		// 通知ThreadPool注销自己
		if (ret == WAIT_TIMEOUT)
		{																	
			mpThreadPool->removeHangThread(this);							
			return false;
		}
		else if(ret != WAIT_OBJECT_0)
		{
			// ERROR_MSG(fmt::format("TPThread::onWaitCondSignal: WaitForSingleObject is error, ret={0}\n", ret));
		}
	}	
#else		
	if(mThreadWaitSecond <= 0)
	{
		lock();
		mState = ThreadState_Sleep;
		pthread_cond_wait(&mCond, &mMutex);
		unlock();
	}
	else
	{
		struct timeval now;
		struct timespec timeout;			
		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec + mThreadWaitSecond;
		timeout.tv_nsec = now.tv_usec * 1000;

		lock();
		mState = ThreadState_Sleep;
		int ret = pthread_cond_timedwait(&mCond, &mMutex, &timeout);
		unlock();

		// 如果是因为超时了， 说明这个线程很久没有被用到， 我们应该注销这个线程。
		if (ret == ETIMEDOUT)
		{
			// 通知ThreadPool注销自己
			mpThreadPool->removeHangThread(this);
			return false;
		}
		else if(ret != 0)
		{
			// ERROR_MSG(fmt::format("TPThread::onWaitCondSignal: pthread_cond_timedwait is error, {0}\n", kbe_strerror()));
		}
	}
#endif
	return true;
}

TPTask* TPThread::tryGetTask(void)
{
	return mpThreadPool->popbufferTask();
}


#if PLATFORM == PLATFORM_WIN32
unsigned __stdcall TPThread::threadFunc(void *arg)
#else	
void* TPThread::threadFunc(void* arg)
#endif
{
	TPThread *tptd = static_cast<TPThread *>(arg);
	ThreadPool *pThreadPool = tptd->threadPool();

	bool isRun = true;
	tptd->resetDoneTasks();

#if PLATFORM == PLATFORM_WIN32
#else			
	pthread_detach(pthread_self());
#endif

	tptd->onStart();

	while(isRun)
	{
		if(tptd->task() != NULL)
		{
			isRun = true;
		}
		else
		{
			tptd->resetDoneTasks();
			isRun = tptd->onWaitCondSignal();
		}

		if(!isRun || pThreadPool->isDestroyed())
		{
			if(!pThreadPool->hasThread(tptd))
				tptd = NULL;

			goto __THREAD_END__;
		}

		TPTask * task = tptd->task();
		if(task == NULL)
			continue;

		tptd->mState = ThreadState_Busy;

		while(task && !tptd->threadPool()->isDestroyed())
		{
			tptd->incDoneTasks();
			tptd->onProcessTaskStart(task);
			tptd->processTask(task); // 处理该任务
			tptd->onProcessTaskEnd(task);

			TPTask * task1 = tptd->tryGetTask(); // 尝试继续从任务队列里取出一个繁忙的未处理的任务

			if(!task1)
			{
				tptd->onTaskCompleted();
				break;
			}
			else
			{
				pThreadPool->addFiniTask(task);
				task = task1;
				tptd->task(task1);
			}
		}
	}

__THREAD_END__:
	if(tptd)
	{
		TPTask * task = tptd->task();
		if(task)
		{
// 			WARNING_MSG(fmt::format("TPThread::threadFunc: task {0:p} not finish, thread.{1:p} will exit.\n", 
// 				(void*)task, (void*)tptd));

			delete task;
		}

		tptd->onEnd();
		tptd->mState = ThreadState_End;
		tptd->resetDoneTasks();
	}

#if PLATFORM == PLATFORM_WIN32
	return 0;
#else	
	pthread_exit(NULL);
	return NULL;
#endif		
}


ThreadPool::ThreadPool()
:mIsInitialize(false)
,mBufferedTaskList()
,mFinishedTaskList()
,mFiniTaskListCount(0)
,mBufferedTaskListMutex()
,mThreadStateListMutex()
,mFinishedTaskListMutex()
,mBusyThreadList()
,mFreeThreadList()
,mAllThreadList()
,mMaxThreadCount(0)
,mExtraNewAddThreadCount(0)
,mCurrentThreadCount(0)
,mCurrentFreeThreadCount(0)
,mNormalThreadCount(0)
,mIsDestroyed(false)
{		
	THREAD_MUTEX_INIT(mThreadStateListMutex);	
	THREAD_MUTEX_INIT(mBufferedTaskListMutex);
	THREAD_MUTEX_INIT(mFinishedTaskListMutex);
}

ThreadPool::~ThreadPool()
{
	Assert(mIsDestroyed && mAllThreadList.size() == 0);
}

void ThreadPool::finalise()
{
	destroy();
}

void ThreadPool::destroy()
{
	mIsDestroyed = true;

	int itry = 0;
	while(true)
	{
		sleep(300);
		itry++;

		std::string taskaddrs = "";
		THREAD_MUTEX_LOCK(mThreadStateListMutex);

		int count = mAllThreadList.size();
		std::list<TPThread *>::iterator itr = mAllThreadList.begin();
		for(; itr != mAllThreadList.end(); ++itr)
		{
			if((*itr))
			{
				if((*itr)->state() != TPThread::ThreadState_End)
				{
					(*itr)->sendCondSignal();
					// taskaddrs += (fmt::format("{0:p},", (void *)(*itr)));
				}
				else
				{
					count--;
				}
			}
		}

		THREAD_MUTEX_UNLOCK(mThreadStateListMutex);

		if(count <= 0)
		{
			break;
		}
		else
		{
			// WARNING_MSG(fmt::format("ThreadPool::destroy(): waiting for thread({0})[{1}], try={2}\n", count, taskaddrs, itry));
		}
	}

	THREAD_MUTEX_LOCK(mThreadStateListMutex);
	sleep(100);
	std::list<TPThread*>::iterator itr = mAllThreadList.begin();
	for(; itr != mAllThreadList.end(); ++itr)
	{
		if((*itr))
		{
			delete (*itr);
			(*itr) = NULL;
		}
	}
	mAllThreadList.clear();
	THREAD_MUTEX_UNLOCK(mThreadStateListMutex);

	THREAD_MUTEX_LOCK(mFinishedTaskListMutex);
	if(mFinishedTaskList.size() > 0)
	{
// 		WARNING_MSG(fmt::format("ThreadPool::~ThreadPool(): Discarding {0} finished tasks.\n",
// 			mFinishedTaskList.size()));

		std::list<TPTask*>::iterator finiiter  = mFinishedTaskList.begin();
		for(; finiiter != mFinishedTaskList.end(); ++finiiter)
		{
			delete (*finiiter);
		}

		mFinishedTaskList.clear();
		mFiniTaskListCount = 0;
	}
	THREAD_MUTEX_UNLOCK(mFinishedTaskListMutex);

	THREAD_MUTEX_LOCK(mBufferedTaskListMutex);
	if(mBufferedTaskList.size() > 0)
	{
// 		WARNING_MSG(fmt::format("ThreadPool::~ThreadPool(): Discarding {0} buffered tasks.\n", 
// 			mBufferedTaskList.size()));

		while(mBufferedTaskList.size() > 0)
		{
			TPTask* tptask = mBufferedTaskList.front();
			mBufferedTaskList.pop();
			delete tptask;
		}
	}
	THREAD_MUTEX_UNLOCK(mBufferedTaskListMutex);

	THREAD_MUTEX_DELETE(mThreadStateListMutex);
	THREAD_MUTEX_DELETE(mBufferedTaskListMutex);
	THREAD_MUTEX_DELETE(mFinishedTaskListMutex);

	// DEBUG_MSG("ThreadPool::destroy(): successfully!\n");
}

bool ThreadPool::createThreadPool(uint32 inewThreadCount, uint32 inormalMaxThreadCount, uint32 imaxThreadCount)
{
	assert(!mIsInitialize);

	mExtraNewAddThreadCount = inewThreadCount;
	mNormalThreadCount = inormalMaxThreadCount;
	mMaxThreadCount = imaxThreadCount;

	for(uint32 i = 0; i < mNormalThreadCount; ++i)
	{
		TPThread* tptd = createThread(0);

		if(!tptd)
		{
			// ERROR_MSG("ThreadPool::createThreadPool: create is error! \n");
			return false;
		}

		mCurrentFreeThreadCount++;	
		mCurrentThreadCount++;
		mFreeThreadList.push_back(tptd);
		mAllThreadList.push_back(tptd);
	}

	mIsInitialize = true;
	sleep(100);
	return true;
}

TPThread* ThreadPool::createThread(int threadWaitSecond)
{
	TPThread* tptd = new TPThread(this, threadWaitSecond);
	tptd->createThread();
	return tptd;
}

void ThreadPool::bufferTask(TPTask* tptask)
{
	THREAD_MUTEX_LOCK(mBufferedTaskListMutex);

	mBufferedTaskList.push(tptask);

	size_t size = mBufferedTaskList.size();
	if(size > THREAD_BUSY_SIZE)
	{
		// WARNING_MSG(fmt::format("ThreadPool::bufferTask: task buffered({0})!\n", size));
	}

	THREAD_MUTEX_UNLOCK(mBufferedTaskListMutex);
}

TPTask* ThreadPool::popbufferTask(void)
{
	TPTask* tptask = NULL;
	THREAD_MUTEX_LOCK(mBufferedTaskListMutex);

	size_t size = mBufferedTaskList.size();
	if(size > 0)
	{
		tptask = mBufferedTaskList.front();
		mBufferedTaskList.pop();

		if(size > THREAD_BUSY_SIZE)
		{
			// WARNING_MSG(fmt::format("ThreadPool::popbufferTask: task buffered({0})!\n", size));
		}
	}

	THREAD_MUTEX_UNLOCK(mBufferedTaskListMutex);	

	return tptask;
}

bool ThreadPool::addFreeThread(TPThread* tptd)
{
	THREAD_MUTEX_LOCK(mThreadStateListMutex);

	std::list<TPThread *>::iterator itr;
	itr = find(mBusyThreadList.begin(), mBusyThreadList.end(), tptd);

	if(itr != mBusyThreadList.end())
	{
		mBusyThreadList.erase(itr);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(mThreadStateListMutex);

		// ERROR_MSG(fmt::format("ThreadPool::addFreeThread: busyThreadList_ not found thread.{0}\n", (uint32)tptd->id()));

		delete tptd;
		return false;
	}

	mFreeThreadList.push_back(tptd);
	mCurrentFreeThreadCount++;

	THREAD_MUTEX_UNLOCK(mThreadStateListMutex);
	return true;
}

bool ThreadPool::addBusyThread(TPThread* tptd)
{
	THREAD_MUTEX_LOCK(mThreadStateListMutex);
	std::list<TPThread*>::iterator itr;
	itr = find(mFreeThreadList.begin(), mFreeThreadList.end(), tptd);

	if(itr != mFreeThreadList.end())
	{
		mFreeThreadList.erase(itr);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(mThreadStateListMutex);
// 		ERROR_MSG(fmt::format("ThreadPool::addBusyThread: freeThreadList_ not "
// 			"found thread.{0}\n",
// 			(uint32)tptd->id()));

		delete tptd;
		return false;
	}

	mBusyThreadList.push_back(tptd);
	--mCurrentFreeThreadCount;
	THREAD_MUTEX_UNLOCK(mThreadStateListMutex);		

	return true;
}

void ThreadPool::addFiniTask(TPTask* tptask)
{ 
	THREAD_MUTEX_LOCK(mFinishedTaskListMutex);
	mFinishedTaskList.push_back(tptask); 
	++mFiniTaskListCount;
	THREAD_MUTEX_UNLOCK(mFinishedTaskListMutex);	
}

bool ThreadPool::removeHangThread(TPThread* tptd)
{
	THREAD_MUTEX_LOCK(mThreadStateListMutex);
	std::list<TPThread*>::iterator itr, itr1;
	itr = find(mFreeThreadList.begin(), mFreeThreadList.end(), tptd);
	itr1 = find(mAllThreadList.begin(), mAllThreadList.end(), tptd);

	if(itr != mFreeThreadList.end() && itr1 != mAllThreadList.end())
	{
		mFreeThreadList.erase(itr);
		mAllThreadList.erase(itr1);
		--mCurrentThreadCount;
		--mCurrentFreeThreadCount;

// 		INFO_MSG(fmt::format("ThreadPool::removeHangThread: thread.{0} is destroy. "
// 			"currentFreeThreadCount:{1}, currentThreadCount:{2}\n",
// 			(uint32)tptd->id(), mCurrentFreeThreadCount, mCurrentThreadCount));

		SafeDelete(tptd);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(mThreadStateListMutex);		

// 		ERROR_MSG(fmt::format("ThreadPool::removeHangThread: not found thread.{0}\n", 
// 			(uint32)tptd->id()));

		return false;
	}

	THREAD_MUTEX_UNLOCK(mThreadStateListMutex);		
	return true;		
}

void ThreadPool::onMainThreadTick()
{
	std::vector<TPTask *> finitasks;

	THREAD_MUTEX_LOCK(mFinishedTaskListMutex);

	if(mFinishedTaskList.size() == 0)
	{
		THREAD_MUTEX_UNLOCK(mFinishedTaskListMutex);	
		return;
	}

	std::copy(mFinishedTaskList.begin(), mFinishedTaskList.end(), std::back_inserter(finitasks));   
	mFinishedTaskList.clear();
	THREAD_MUTEX_UNLOCK(mFinishedTaskListMutex);	

	std::vector<TPTask *>::iterator finiiter  = finitasks.begin();

	for(; finiiter != finitasks.end(); )
	{
		TPTask::TPTaskState state = (*finiiter)->presentMainThread();

		switch(state)
		{
		case TPTask::TPTask_Completed:
			delete (*finiiter);
			finiiter = finitasks.erase(finiiter);
			--mFiniTaskListCount;
			break;
		case TPTask::TPTask_ContinueChildThread:
			this->addTask((*finiiter));
			finiiter = finitasks.erase(finiiter);
			--mFiniTaskListCount;
			break;
		case TPTask::TPTask_ContinueMainThread:
			THREAD_MUTEX_LOCK(mFinishedTaskListMutex);
			mFinishedTaskList.push_back((*finiiter));
			THREAD_MUTEX_UNLOCK(mFinishedTaskListMutex);	
			++finiiter;
			break;
		default:
			Assert(false);
			break;
		};
	}
}

bool ThreadPool::hasThread(TPThread* pTPThread)
{
	bool ret = true;

	THREAD_MUTEX_LOCK(mThreadStateListMutex);

	std::list<TPThread*>::iterator itr1 = find(mAllThreadList.begin(), mAllThreadList.end(), pTPThread);
	if(itr1 == mAllThreadList.end())
		ret = false;

	THREAD_MUTEX_UNLOCK(mThreadStateListMutex);

	return ret;
}

std::string ThreadPool::printThreadWorks()
{
	std::string ret;

	THREAD_MUTEX_LOCK(mThreadStateListMutex);
	int i = 0;
	std::list<TPThread*>::iterator itr = mBusyThreadList.begin();
	for(; itr != mBusyThreadList.end(); ++itr)
	{
		// ret += (fmt::format("{0:p}:({1}), ", (void*)(*itr), (*itr)->printWorkState()));
		i++;

		if(i > 1024)
			break;
	}

	THREAD_MUTEX_UNLOCK(mThreadStateListMutex);		
	return ret;
}

bool ThreadPool::addTask(TPTask* tptask)
{
	THREAD_MUTEX_LOCK(mThreadStateListMutex);
	if(mCurrentFreeThreadCount > 0)
	{
		std::list<TPThread *>::iterator itr = mFreeThreadList.begin();
		TPThread* tptd = (TPThread *)(*itr);
		mFreeThreadList.erase(itr);
		mBusyThreadList.push_back(tptd);
		--mCurrentFreeThreadCount;
		
		//INFO_MSG("ThreadPool::currFree:%d, currThreadCount:%d, busy:[%d]\n",
		//		 currentFreeThreadCount_, currentThreadCount_, busyThreadList_count_);
		
		tptd->task(tptask);												// 给线程设置新任务	
		
#if PLATFORM == PLATFORM_WIN32
		if(tptd->sendCondSignal()== 0){
#else
		if(tptd->sendCondSignal()!= 0){
#endif
			// ERROR_MSG("ThreadPool::addTask: pthread_cond_signal is error!\n");
			THREAD_MUTEX_UNLOCK(mThreadStateListMutex);
			return false;
		}
		
		THREAD_MUTEX_UNLOCK(mThreadStateListMutex);
		return true;
	}
	
	bufferTask(tptask);
	
	if(isThreadCountMax())
	{
		THREAD_MUTEX_UNLOCK(mThreadStateListMutex);

		//WARNING_MSG(fmt::format("ThreadPool::addTask: can't createthread, the poolsize is full({}).\n,
		//	maxThreadCount_));

		return false;
	}

	for(uint32 i = 0; i < mExtraNewAddThreadCount; ++i)
	{
		TPThread* tptd = createThread(300);									// 设定5分钟未使用则退出的线程
		if(!tptd)
		{
#if PLATFORM == PLATFORM_WIN32		
			// ERROR_MSG("ThreadPool::addTask: the ThreadPool create thread is error! ... \n");
#else
// 			ERROR_MSG(fmt::format("ThreadPool::addTask: the ThreadPool create thread is error:{0}\n", 
// 				__strerror()));
#endif				
		}

		mAllThreadList.push_back(tptd);										// 所有的线程列表
		mFreeThreadList.push_back(tptd);									// 闲置的线程列表
		++mCurrentThreadCount;
		++mCurrentFreeThreadCount;	
		
	}
	
	// INFO_MSG(fmt::format("ThreadPool::addTask: new Thread, currThreadCount: {0}\n", mCurrentThreadCount));

	THREAD_MUTEX_UNLOCK(mThreadStateListMutex);
	return true;
}
