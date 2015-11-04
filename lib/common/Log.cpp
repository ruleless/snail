#include "Log.h"
#include <string>

// 追踪日志
void traceLn(const char *logMsg)
{
	LogSystem::getSingleton().log(LogSystem::Trace, std::string(logMsg)+"\n");
}

// 警告日志
void waringLn(const char *logMsg)
{
	LogSystem::getSingleton().log(LogSystem::Warning, std::string(logMsg)+"\n");
}

// 错误日志
void errorLn(const char *logMsg)
{
	LogSystem::getSingleton().log(LogSystem::Error, std::string(logMsg)+"\n");
}
