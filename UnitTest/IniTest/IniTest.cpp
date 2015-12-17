#include "IniTest.h"
#include "thread/ThreadPool.h"
#include "common/Ini.h"
#include "common/Buffer.h"
#include "common/timestamp.h"

CPPUNIT_TEST_SUITE_REGISTRATION(IniTest);

IniTest::IniTest()
{
}

IniTest::~IniTest()
{
}

void IniTest::setUp()
{
}

void IniTest::tearDown()
{
}

void IniTest::testIniWrite()
{
	uint64 begT = timestamp();
	Ini ini = Ini("utest.ini");
	uint64 endT = timestamp();
	printf("testIniWrite! ini read time:%.3lfms\n", (endT-begT)/1000.);

	begT = timestamp();
	static const int s_testTime = 200;
	for (int i = 0; i < s_testTime; ++i)
	{
		char sec[MAX_BUF];
		__snprintf(sec, sizeof(sec), "sec%d", i);
		for (int i = 0; i < s_testTime; ++i)
		{
			char key[MAX_BUF];
			__snprintf(key, sizeof(key), "key%d", i);
			ini.setInt(sec, key, i);
		}
	}
	endT = timestamp();
	printf("testIniWrite! ini set time:%.3lfms\n", (endT-begT)/1000.);
}

void IniTest::testIniRead()
{
	uint64 begT = timestamp();
	Ini ini = Ini("utest.ini");
	uint64 endT = timestamp();
	printf("testIniRead! ini read time:%.3lfms\n", (endT-begT)/1000.);

	begT = timestamp();
	static const int s_testTime = 200;
	for (int i = 0; i < s_testTime; ++i)
	{
		char sec[MAX_BUF];
		__snprintf(sec, sizeof(sec), "sec%d", i);
		for (int i = 0; i < s_testTime; ++i)
		{
			char key[MAX_BUF], val[MAX_BUF];
			__snprintf(key, sizeof(key), "key%d", i);
			CPPUNIT_ASSERT(ini.getInt(sec, key) == i);
		}
	}
	endT = timestamp();
	printf("testIniRead! ini get time:%.3lfms\n", (endT-begT)/1000.);
}