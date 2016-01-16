#ifndef __THREADTASK_H__
#define __THREADTASK_H__

#include "common.h"
#include "Task.h"

class TPTask : public Task
{
public:
	enum TPTaskState
	{
		TPTask_Completed = 0, // һ�������Ѿ����
		TPTask_ContinueMainThread = 1, // ���������߳�ִ��
		TPTask_ContinueChildThread = 2, // ���������߳�ִ��
	};

	virtual TPTask::TPTaskState presentMainThread() { return TPTask::TPTask_Completed; }
};

#endif // __THREADTASK_H__
