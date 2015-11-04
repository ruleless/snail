#include "LogTest.h"
#include "thread/ThreadPool.h"
#include "common/Log.h"
#include "common/LogSystem.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LogTest);

LogTest::LogTest()
		:mpThreadPool(new ThreadPool())
{
}

LogTest::~LogTest()
{
	delete mpThreadPool;
}

void LogTest::setUp()
{
	CPPUNIT_ASSERT(mpThreadPool->createThreadPool(1, 30, 50));

	new LogSystem(mpThreadPool);
	CPPUNIT_ASSERT(LogSystem::getSingleton().start());
}

void LogTest::tearDown()
{
	LogSystem::getSingleton().stop();

	mpThreadPool->destroy();
	delete LogSystem::getSingletonPtr();
}

void LogTest::printLog()
{
	for (int i = 0; i < 100; ++i)
	{
		traceLn("I am white");
		waringLn("I am yellow");
		errorLn("I am red");
		mpThreadPool->onMainThreadTick();
	}
}
