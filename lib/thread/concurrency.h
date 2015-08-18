#ifndef __CONCURRENCY_H__
#define __CONCURRENCY_H__

#include "common/platform.h"

extern void (*pMainThreadIdleStartCallback)();
extern void (*pMainThreadIdleEndCallback)();

// 主线程处于空闲时触发
inline void onStartMainThreadIdling()
{
	if(pMainThreadIdleStartCallback)
		(*pMainThreadIdleStartCallback)();
}

// 主线程结束空闲开始繁忙时触发
inline void onEndMainThreadIdling()
{
	if(pMainThreadIdleEndCallback)
		(*pMainThreadIdleEndCallback)();
}

// 设置回调函数
inline void setMainThreadIdleCallbacks(void (*pStartCallback)(), void (*pEndCallback)())
{
	pMainThreadIdleStartCallback = pStartCallback;
	pMainThreadIdleEndCallback = pEndCallback;
}

#endif // __CONCURRENCY_H__
