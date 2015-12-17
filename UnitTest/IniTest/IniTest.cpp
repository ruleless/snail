#include "IniTest.h"
#include "thread/ThreadPool.h"
#include "common/Ini.h"
#include "common/Buffer.h"

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
	Ini ini = Ini("utest.ini");

	static const int s_testTime = 1000;
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
}

void IniTest::testIniRead()
{
	Ini ini = Ini("utest.ini");
	static const int s_testTime = 1000;
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
}