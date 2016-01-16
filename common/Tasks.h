#ifndef __TASKS_H__
#define __TASKS_H__

#include "common/Task.h"
#include "common/common.h"

class Tasks
{
public:
	Tasks();
	~Tasks();

	void add(Task * pTask);
	bool cancel(Task * pTask);
	void process();
private:
	typedef std::vector<Task *> Container;
	Container mTasks;
};

#endif // __TASKS_H__
