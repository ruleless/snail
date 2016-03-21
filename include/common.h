#ifndef __COMMON_H__
#define __COMMON_H__

#include "platform.h"

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a, b) ((a)>(b)?(a):(b))
#endif

#define PRINT_INTVAL(val) printf("%s:%d\n", #val, val)

#ifdef _INLINE
#define INLINE inline
#else
#define INLINE
#endif

#define Assert assert

// 安全删除和释放
#define SafeDelete(ptr)      if ((ptr)) {delete (ptr); (ptr) = 0;}
#define SafeDeleteArray(ptr) if ((ptr)) {delete[] (ptr); (ptr) = 0;}
#define SafeRelease(ptr)     if ((ptr)) {(ptr)->Release(); (ptr) = 0;}

#define SUPPORT_TRACE

#ifdef _UNICODE
#	define _UTF8
#endif

#endif
