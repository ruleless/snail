#ifndef __THREAD_H__
#define __THREAD_H__

#include <unistd.h>
#include <pthread.h>

class Thread
{
public:
	enum Status
	{
		S_Ready,
		S_Running,
		S_Exiting,
	};
	
	Thread();
	virtual ~Thread();

	void start();
	void wait(void *status);

	Status getStatus() const;
	void _setStatus(Status s);
	pthread_t getThreadID() const;

	virtual void run();
protected:
	pthread_t mTID;
	Status mStatus;
};

#endif
