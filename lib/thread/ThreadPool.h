#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include "common/common.h"
#include "common/Tasks.h"
#include "common/Singleton.h"
#include "thread/ThreadTask.h"

// windows include	
#if PLATFORM == PLATFORM_WIN32
#include <windows.h> // for HANDLE
#include <process.h> // for _beginthread()	
#else
// linux include
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>	
#endif

// 线程池活动线程大于这个数目则处于繁忙状态
#define THREAD_BUSY_SIZE 32

class ThreadPool;
class TPThread
{
public:
	friend class ThreadPool;

	enum EThreadState
	{
		ThreadState_Stop = -1,
		ThreadState_Sleep = 0,
		ThreadState_Busy = 1,
		ThreadState_End = 2
	};

	TPThread(ThreadPool* threadPool, int threadWaitSecond = 0)
		:mThreadWaitSecond(threadWaitSecond)
		,mpCurrTask(NULL)
		,mpThreadPool(threadPool)
	{
		mState = ThreadState_Sleep;
		initCond();
		initMutex();
	}
		
	virtual ~TPThread()
	{
		deleteCond();
		deleteMutex();
	}

	THREAD_ID createThread(void);
	bool join(void);

	void onTaskCompleted(void);

	// 线程通知 等待条件信号
	bool onWaitCondSignal(void);

	virtual TPTask* tryGetTask(void);

#if PLATFORM == PLATFORM_WIN32
	static unsigned __stdcall threadFunc(void *arg);
#else	
	static void* threadFunc(void* arg);
#endif

	int sendCondSignal(void) { return THREAD_SINGNAL_SET(mCond); }
	
	virtual void onStart() {}
	virtual void onEnd() {}

	virtual void onProcessTaskStart(TPTask* pTask) {}
	virtual void processTask(TPTask* pTask) { pTask->process(); }
	virtual void onProcessTaskEnd(TPTask* pTask) {}

	THREAD_ID id(void) const { return mTid; }
	void id(THREAD_ID tidp) { mTid = tidp; }
	
	virtual void initCond(void) { THREAD_SINGNAL_INIT(mCond); }
	virtual void initMutex(void) { THREAD_MUTEX_INIT(mMutex); }

	virtual void deleteCond(void) { THREAD_SINGNAL_DELETE(mCond); }
	virtual void deleteMutex(void) { THREAD_MUTEX_DELETE(mMutex); }

	virtual void lock(void) { THREAD_MUTEX_LOCK(mMutex); }
	virtual void unlock(void) {	THREAD_MUTEX_UNLOCK(mMutex); }

	TPTask* task(void) const { return mpCurrTask; }
	void task(TPTask* tpt) { mpCurrTask = tpt; }

	int state(void) const { return mState; }

	ThreadPool* threadPool() { return mpThreadPool; }

	virtual std::string printWorkState()
	{
		char buf[128];
		lock();
		sprintf(buf, "%p,%u", mpCurrTask, mDoneTasks);
		unlock();
		return buf;
	}

	void resetDoneTasks() { mDoneTasks = 0; }
	void incDoneTasks() { ++mDoneTasks; }
protected:
	THREAD_ID mTid; // 本线程的ID
	THREAD_SINGNAL mCond;
	THREAD_MUTEX mMutex;

	int mThreadWaitSecond; // 线程空闲状态超过这个秒数则线程退出, 小于0为永久线程(秒单位)
	TPTask *mpCurrTask; // 该线程的当前执行的任务
	ThreadPool *mpThreadPool; // 线程池指针
	EThreadState mState; // 线程状态
	uint32 mDoneTasks; // 线程启动一次在未改变到闲置状态下连续执行的任务计数
};


class ThreadPool
{
public:		
	ThreadPool();
	virtual ~ThreadPool();
	
	void finalise();
	void destroy();
	
	/** 创建线程池
	@param inewThreadCount: 当系统繁忙时线程池会新增加这么多线程（临时）
	@param inormalMaxThreadCount: 线程池会一直保持这么多个数的线程
	@param imaxThreadCount: 线程池最多只能有这么多个线程
	*/
	bool createThreadPool(uint32 inewThreadCount, uint32 inormalMaxThreadCount, uint32 imaxThreadCount);

	virtual TPThread* createThread(int threadWaitSecond = ThreadPool::timeout);

	void bufferTask(TPTask* tptask);
	TPTask* popbufferTask(void);

	bool addFreeThread(TPThread* tptd);
	bool addBusyThread(TPThread* tptd);
	void addFiniTask(TPTask* tptask);

	bool removeHangThread(TPThread* tptd);

	virtual void onMainThreadTick();

	bool hasThread(TPThread* pTPThread);

	std::string printThreadWorks();

	bool addTask(TPTask* tptask);

	bool addBackgroundTask(TPTask* tptask) { return addTask(tptask); }
	bool pushTask(TPTask* tptask) { return addTask(tptask); }

	uint32 currentThreadCount(void) const { return mCurrentThreadCount; }
	uint32 currentFreeThreadCount(void) const { return mCurrentFreeThreadCount; }
	bool isThreadCountMax(void) const { return mCurrentThreadCount >= mMaxThreadCount; }
	
	bool isBusy(void) const { return mBufferedTaskList.size() > THREAD_BUSY_SIZE; }

	bool isInitialize(void) const { return mIsInitialize; }
	bool isDestroyed() const { return mIsDestroyed; }

	uint32 bufferTaskSize() const { return mBufferedTaskList.size(); }
	std::queue<TPTask*>& bufferedTaskList() { return mBufferedTaskList; }

	void lockBufferedTaskList() { THREAD_MUTEX_LOCK(mBufferedTaskListMutex); }
	void unlockBufferedTaskList() { THREAD_MUTEX_UNLOCK(mBufferedTaskListMutex); }

	uint32 finiTaskSize() const { return mFiniTaskListCount; }

	virtual std::string name() const{ return "ThreadPool"; }
public:
	static int timeout;
protected:
	bool mIsInitialize;
	bool mIsDestroyed;
	
	std::queue<TPTask *> mBufferedTaskList; // 系统处于繁忙时还未处理的任务列表
	std::list<TPTask *> mFinishedTaskList; // 已经完成的任务列表
	size_t mFiniTaskListCount;

	THREAD_MUTEX mBufferedTaskListMutex; // 处理mBufferedTaskList互斥锁
	THREAD_MUTEX mThreadStateListMutex; // 处理mBufferedTaskList and mFreeThreadList互斥锁
	THREAD_MUTEX mFinishedTaskListMutex; // 处理mFinishedTaskList互斥锁
	
	std::list<TPThread *> mBusyThreadList; // 繁忙的线程列表
	std::list<TPThread *> mFreeThreadList; // 闲置的线程列表
	std::list<TPThread *> mAllThreadList; // 所有的线程列表

	uint32 mMaxThreadCount; // 最大线程总数
	uint32 mExtraNewAddThreadCount; // 如果mNormalThreadCount不足够使用则会新创建这么多线程
	uint32 mCurrentThreadCount; // 当前线程数
	uint32 mCurrentFreeThreadCount; // 当前闲置的线程数
	uint32 mNormalThreadCount; // 标准状态下的线程总数 即：默认情况下一启动服务器就开启这么多线程，如果线程不足够，则会新创建一些线程， 最大能够到mMaxThreadCount
};

#endif // __THREADPOOL_H__
