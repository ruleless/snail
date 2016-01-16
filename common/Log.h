#ifndef __LOG_H__
#define __LOG_H__

#include "LogSystem.h"

// 追踪日志
extern void traceLn(const char *logMsg);

// 警告日志
extern void waringLn(const char *logMsg);

// 错误日志
extern void errorLn(const char *logMsg);

#endif
