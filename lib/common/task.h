#ifndef __TASK_H__
#define __TASK_H__

class Task
{
public:
	virtual ~Task() {}
	virtual bool process() = 0;
};

#endif // __TASK_H__
