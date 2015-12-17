#ifndef __INITEST_H__
#define __INITEST_H__

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

class ThreadPool;
class IniTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(IniTest);
	CPPUNIT_TEST(testIniWrite);
	CPPUNIT_TEST(testIniRead);
	CPPUNIT_TEST_SUITE_END();
  public:
    IniTest();
    virtual ~IniTest();

	virtual void setUp();

	virtual void tearDown();

	void testIniWrite();
	void testIniRead();
};

#endif // __INITEST_H__