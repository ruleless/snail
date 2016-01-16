#include "Tasks.h"
#include "ThreadGuard.h"

Tasks::Tasks() : mTasks()
{
}

Tasks::~Tasks()
{
}

void Tasks::add( Task * pTask )
{
	mTasks.push_back( pTask );
}

bool Tasks::cancel( Task * pTask )
{
	Container::iterator iter = std::find(mTasks.begin(), mTasks.end(), pTask);
	if (iter != mTasks.end())
	{
		mTasks.erase( iter );
		return true;
	}

	return false;
}

void Tasks::process()
{
	Container::iterator iter = mTasks.begin();

	while (iter != mTasks.end())
	{
		Task * pTask = *iter;
		if(!pTask->process())
			iter = mTasks.erase(iter);
		else
			++iter;
	}
}
