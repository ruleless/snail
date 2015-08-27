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

// �̳߳ػ�̴߳��������Ŀ���ڷ�æ״̬
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

	// �߳�֪ͨ �ȴ������ź�
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
	THREAD_ID mTid; // ���̵߳�ID
	THREAD_SINGNAL mCond;
	THREAD_MUTEX mMutex;

	int mThreadWaitSecond; // �߳̿���״̬��������������߳��˳�, С��0Ϊ�����߳�(�뵥λ)
	TPTask *mpCurrTask; // ���̵߳ĵ�ǰִ�е�����
	ThreadPool *mpThreadPool; // �̳߳�ָ��
	EThreadState mState; // �߳�״̬
	uint32 mDoneTasks; // �߳�����һ����δ�ı䵽����״̬������ִ�е��������
};


class ThreadPool
{
public:		
	ThreadPool();
	virtual ~ThreadPool();
	
	void finalise();
	void destroy();
	
	/** �����̳߳�
	@param inewThreadCount: ��ϵͳ��æʱ�̳߳ػ���������ô���̣߳���ʱ��
	@param inormalMaxThreadCount: �̳߳ػ�һֱ������ô��������߳�
	@param imaxThreadCount: �̳߳����ֻ������ô����߳�
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
	
	std::queue<TPTask *> mBufferedTaskList; // ϵͳ���ڷ�æʱ��δ����������б�
	std::list<TPTask *> mFinishedTaskList; // �Ѿ���ɵ������б�
	size_t mFiniTaskListCount;

	THREAD_MUTEX mBufferedTaskListMutex; // ����mBufferedTaskList������
	THREAD_MUTEX mThreadStateListMutex; // ����mBufferedTaskList and mFreeThreadList������
	THREAD_MUTEX mFinishedTaskListMutex; // ����mFinishedTaskList������
	
	std::list<TPThread *> mBusyThreadList; // ��æ���߳��б�
	std::list<TPThread *> mFreeThreadList; // ���õ��߳��б�
	std::list<TPThread *> mAllThreadList; // ���е��߳��б�

	uint32 mMaxThreadCount; // ����߳�����
	uint32 mExtraNewAddThreadCount; // ���mNormalThreadCount���㹻ʹ������´�����ô���߳�
	uint32 mCurrentThreadCount; // ��ǰ�߳���
	uint32 mCurrentFreeThreadCount; // ��ǰ���õ��߳���
	uint32 mNormalThreadCount; // ��׼״̬�µ��߳����� ����Ĭ�������һ�����������Ϳ�����ô���̣߳�����̲߳��㹻������´���һЩ�̣߳� ����ܹ���mMaxThreadCount
};

#endif // __THREADPOOL_H__
