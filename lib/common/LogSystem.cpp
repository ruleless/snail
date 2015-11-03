#include "LogSystem.h"
#include "thread/ThreadPool.h"

LogSystem::LogSystem(ThreadPool *pThreadPool)
		:TPTask()
		,mrpos(0)
		,mwpos(0)
		,mStarted(false)
		,mLogLevel(MaxLevel)
		,mpThreadPool(pThreadPool)
{
	assert(mpThreadPool);

	memset(mLogPipe, 0, sizeof(mLogPipe));
	memset(mPipeEnabled, 0, sizeof(mPipeEnabled));
}

LogSystem::~LogSystem()
{
}

bool LogSystem::start()
{
	if (!mStarted)
	{
		mrpos = mwpos = 0;
		if (sem_init(&mEmpty, 0, MAX_LOGMSG_LEN) < 0)
		{
			return false;
		}
		if (sem_init(&mStored, 0, 0) < 0)
		{
			sem_destroy(&mEmpty);
			return false;
		}

		for (int i = 0; i < LogPipe_Max; ++i)
		{
			mLogPipe[i] = ;
			mPipeEnabled[i] = true;
		}

		mStarted = true;
		mLogLevel = MaxLevel;

		mpThreadPool->addTask(this);
	}
	return true;
}

void LogSystem::stop()
{
	if (mStarted)
	{
		mStarted = false;

		// 保证所有log日志都被输出
		int nEmpty = 0;
		sem_getvalue(&mEmpty, &nEmpty);
		while (MAX_LOGMSG_LEN != nEmpty)
			sem_getvalue(&mEmpty, &nEmpty);

		for (int i = 0; i < LogPipe_Max; ++i)
		{
			if (mLogPipe[i])
			{
				delete mLogPipe[i];
				mLogPipe[i] = NULL;
			}
		}

		sem_destroy(&mEmpty);
		sem_destroy(&mStored);
	}
}

void LogSystem::log(ELevel level, std::string msg)
{
	if (mStarted && level <= mLogLevel)
	{
		if (sem_wait(&mEmpty) == 0)
		{
			static std::string s_logLevelStr[MaxLevel] = {
				"Trace", "Warning", "Error",
			};

			static int s_timeStrLen = 100;
			static char s_timeStr[s_timeStrLen] = {0};
			time_t nowtime;
			struct tm *timeinfo;
			time(&nowtime);
			timeinfo = localtime(&nowtime);
			snprintf(s_timeStr, s_timeStrLen, "[%4d-%2d-%2d %2d:%2d]  ",
					 timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
					 timeinfo->tm_hour, timeinfo.tm_sec);

			mLogMsg[mwpos].clear();
			mLogMsg[mwpos] += s_timeStr;
			mLogMsg[mwpos] += s_logLevelStr[level];
			mLogMsg[mwpos] += msg;

			mwpos = (mwpos+1)%MAX_LOGMSG_LEN;

			sem_post(&mStored);
		}
	}
}

bool LogSystem::process()
{
	while (sem_trywait(&mStored) == 0)
	{
		for (int i = 0; i < LogPipe_Max; ++i)
		{
			if (mLogPipe[i] && mPipeEnabled[i])
			{
				mLogPipe[i]->output(mLogMsg[mrpos]);
			}
		}
		mrpos = (mrpos+1)%MAX_LOGMSG_LEN;

		sem_post(&mEmpty);
	}
	return true;
}

TPTask::TPTaskState LogSystem::presentMainThread()
{
	if (mStarted)
		return TPTask::TPTask_ContinueChildThread;

	return TPTask::TPTask_Completed;
}
