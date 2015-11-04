#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

class ThreadPool;
class LogTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(LogTest);
	CPPUNIT_TEST(printLog);
	CPPUNIT_TEST_SUITE_END();
  public:
    LogTest();
    virtual ~LogTest();

	virtual void setUp();

	virtual void tearDown();

	void printLog();
  private:
	ThreadPool *mpThreadPool;
};
