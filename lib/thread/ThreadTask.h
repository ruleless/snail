#ifndef __THREADTASK_H__
#define __THREADTASK_H__

#include "common/common.h"
#include "common/task.h"

class TPTask : public Task
{
public:
	enum TPTaskState
	{
		TPTask_Completed = 0, // 一个任务已经完成
		TPTask_ContinueMainThread = 1, // 继续在主线程执行
		TPTask_ContinueChildThread = 2, // 继续在子线程执行
	};

	virtual TPTask::TPTaskState presentMainThread() { return TPTask::TPTask_Completed; }
};

#endif // __THREADTASK_H__
