#ifndef __CONCURRENCY_H__
#define __CONCURRENCY_H__

#include "common/platform.h"

extern void (*pMainThreadIdleStartCallback)();
extern void (*pMainThreadIdleEndCallback)();

// ���̴߳��ڿ���ʱ����
inline void onStartMainThreadIdling()
{
	if(pMainThreadIdleStartCallback)
		(*pMainThreadIdleStartCallback)();
}

// ���߳̽������п�ʼ��æʱ����
inline void onEndMainThreadIdling()
{
	if(pMainThreadIdleEndCallback)
		(*pMainThreadIdleEndCallback)();
}

// ���ûص�����
inline void setMainThreadIdleCallbacks(void (*pStartCallback)(), void (*pEndCallback)())
{
	pMainThreadIdleStartCallback = pStartCallback;
	pMainThreadIdleEndCallback = pEndCallback;
}

#endif // __CONCURRENCY_H__
