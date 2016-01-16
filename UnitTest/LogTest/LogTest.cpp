#include "LogTest.h"
#include "Trace.h"

CPPUNIT_TEST_SUITE_REGISTRATION(LogTest);

LogTest::LogTest()
{
}

LogTest::~LogTest()
{
}

void LogTest::setUp()
{
	createTrace();
	output2Console();
	output2Html("log.html");
}

void LogTest::tearDown()
{
	closeTrace();
}

void LogTest::printLog()
{
	for (int i = 0; i < 100; ++i)
	{
		InfoLn("Info");
		TraceLn("Trace");
		WarningLn("Waring");
		ErrorLn("Error");
		EmphasisLn("EmphasisLn");

		sleepms(16);
	}
	getchar();
	getchar();
}
