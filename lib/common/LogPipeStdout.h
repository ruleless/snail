#ifndef __LOGPIPESTDOUT_H__
#define __LOGPIPESTDOUT_H__

#include "LogSystem.h"

class LogPipeStdout : public LogPipe
{
  public:
    LogPipeStdout();
    virtual ~LogPipeStdout();

	virtual void output(LogSystem::ELevel level, std::string msg);
};

#endif
