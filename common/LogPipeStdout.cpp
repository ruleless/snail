#include "LogPipeStdout.h"

#define  STD_COLOR_NONE    "\033[0m"
#define  STD_COLOR_YELLOW  "\033[33m"
#define  STD_COLOR_RED     "\033[31m"

LogPipeStdout::LogPipeStdout()
		:LogPipe()
{
}

LogPipeStdout::~LogPipeStdout()
{
}

void LogPipeStdout::output(LogSystem::ELevel level, std::string msg)
{
	switch (level)
	{
	case LogSystem::Trace:
		{
			printf(STD_COLOR_NONE"%s", msg.c_str());
		}
		break;
	case LogSystem::Warning:
		{
			printf(STD_COLOR_YELLOW"%s", msg.c_str());
		}
		break;
	case LogSystem::Error:
		{
			printf(STD_COLOR_RED"%s", msg.c_str());
		}
		break;
	default:
		break;
	}
	printf(STD_COLOR_NONE);
}
