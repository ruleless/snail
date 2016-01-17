#include "common.h"
#include "Trace.h"

#ifdef SUPPORT_TRACE

#if PLATFORM == PLATFORM_WIN32
#	include <windows.h>
#	include <process.h>
#endif

STrace* gTrace = NULL;

STrace::STrace()
		:mSinks()
		,mLevel(levelAll)
		,mHasTime(true)
		,mLimitFrequency(false)
		,mbExit(false)
		,mInited(false)
		,mMsgs1()
		,mMsgs2()
		,mInlist(NULL)
		,mOutlist(NULL)
{
}

STrace::~STrace()
{
	finalise();
}

bool STrace::initialise(int level, bool hasTime)
{
	mLevel = level;
	mHasTime = hasTime;
	mInlist = &mMsgs1;
	mOutlist = &mMsgs2;

#if PLATFORM == PLATFORM_WIN32
	if ((mTid = (THREAD_ID)_beginthreadex(NULL, 0, &STrace::_traceProc, (void *)this, NULL, 0)) == NULL)
	{
		return false;
	}

	InitializeCriticalSection(&mMutex);
	mCond = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == mCond)
	{
		return false;
	}
#else
	if(pthread_create(&mTid, NULL, STrace::_traceProc, (void *)this) != 0)
	{
		return false;
	}

	if (pthread_mutex_init(&mMutex, NULL) != 0)
	{
		return false;
	}
	if (pthread_cond_init(&mCond, NULL) != 0)
	{
		return false;
	}
#endif
	mInited = true;
	return true;
}

void STrace::finalise()
{
	if (mInited)
	{
		mbExit = true;
		THREAD_SINGNAL_SET(mCond);
#if PLATFORM == PLATFORM_WIN32
		::WaitForSingleObject(mTid, INFINITE);
		::CloseHandle(mTid);
#else
		pthread_join(mTid, NULL);
#endif

		mOutlist = mInlist;
		_flushOutlist();

		THREAD_SINGNAL_DELETE(mCond);
		THREAD_MUTEX_DELETE(mMutex);

		for (STrace::ListenerList::iterator it = mSinks.begin(); it != mSinks.end(); ++it)
			delete *it;

		mSinks.clear();

		mInited = false;
	}
}

#if PLATFORM == PLATFORM_WIN32
unsigned __stdcall STrace::_traceProc(void *arg)
#else
		void* STrace::_traceProc(void* arg)
#endif
{
	STrace *pLog = (STrace *)arg;
	while (!pLog->mbExit)
	{
		THREAD_MUTEX_LOCK(pLog->mMutex);
		while (pLog->mInlist->empty() && !pLog->mbExit)
		{
#if PLATFORM == PLATFORM_WIN32
			THREAD_MUTEX_UNLOCK(pLog->mMutex);
			::WaitForSingleObject(pLog->mCond, INFINITE);
			THREAD_MUTEX_LOCK(pLog->mMutex);
#else
			pthread_cond_wait(&pLog->mCond, &pLog->mMutex);
#endif
		}

		STrace::MsgList *tmpList = pLog->mOutlist;
		pLog->mOutlist = pLog->mInlist;
		pLog->mInlist = tmpList;
		THREAD_MUTEX_UNLOCK(pLog->mMutex);

		pLog->_flushOutlist();
	}

	return 0;
}

void STrace::_flushOutlist()
{
	MsgList* outlist = mOutlist;
	if (!outlist->empty())
	{
		char timeStr[32] = {0};
		const char* pTimeStr = 0;
		for (MsgList::iterator i = outlist->begin(); i != outlist->end(); ++i)
		{
			_MSG& msgnode = *i;

			if (msgnode.time)
			{
				struct tm *timeinfo = localtime(&msgnode.time);
				__snprintf(timeStr, sizeof(timeStr), "[%04d/%02d/%d %02d:%02d:%02d]",
						   timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
				pTimeStr = timeStr;
			}
			else
			{
				pTimeStr = 0;
			}

			for (ListenerList::iterator it = mSinks.begin(); it != mSinks.end(); ++it)
			{
				Listener* obj = *it;
				if (obj->getTraceLevel() & msgnode.level)
				{
					obj->onTrace(msgnode.msg.c_str(), pTimeStr, msgnode.level);
				}
			}
		}

		outlist->clear();
	}
}

int STrace::getTraceLevel() const
{
	return mLevel;
}

int STrace::setTraceLevel(int level)
{
	THREAD_MUTEX_LOCK(mMutex);
	int old = mLevel;
	mLevel = level;
	THREAD_MUTEX_UNLOCK(mMutex);

	return old;
}

bool STrace::hasTime(bool b)
{
	THREAD_MUTEX_LOCK(mMutex);
	bool old = mHasTime;
	mHasTime = b;
	THREAD_MUTEX_UNLOCK(mMutex);

	return old;
}

bool STrace::setTraceLimitFrequency(bool limitFrequency)
{
	THREAD_MUTEX_LOCK(mMutex);
	bool old = mLimitFrequency;
	mLimitFrequency = limitFrequency;
	THREAD_MUTEX_UNLOCK(mMutex);

	return old;
}

bool STrace::hasLimitFrequency() const
{
	return mLimitFrequency;
}

void STrace::registerTrace(Listener* sink)
{
	if (NULL == sink)
		return;

	mSinks.remove(sink);
	mSinks.push_back(sink);
}

void STrace::unregisterTrace(Listener* sink)
{
	if (NULL == sink)
		return;

	for (ListenerList::iterator it = mSinks.begin(); it != mSinks.end(); ++it)
	{
		if (*it == sink)
		{
			mSinks.erase(it);
			break;
		}
	}
}

void STrace::output(const char* msg, TraceLevel level)
{
	if (NULL == msg)
		return;

	if ((mLevel & (int)level) == 0)
		return;

#if PLATFORM == PLATFORM_WIN32
#ifdef _DEBUG
	if (::IsDebuggerPresent())
	{
		::OutputDebugString(msg);
	}
#endif
#endif

	THREAD_MUTEX_LOCK(mMutex);

	mInlist->push_back(_MSG());
	_MSG& msgnode = mInlist->back();
	msgnode.level = level;
	msgnode.time = mHasTime ? time(0) : 0;
	msgnode.msg = msg;

	THREAD_MUTEX_UNLOCK(mMutex);

	THREAD_SINGNAL_SET(mCond);
}

#endif
