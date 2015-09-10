#include "algorithm/Queue.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

class QueueTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(QueueTest);
	CPPUNIT_TEST(testQueueOperation);
	CPPUNIT_TEST(_testQueueOperation);
	CPPUNIT_TEST_SUITE_END();
  public:
    QueueTest();
    virtual ~QueueTest();

	virtual void setUp();

	virtual void tearDown();
  protected:
	void testQueueOperation();
	void _testQueueOperation();
  private:
	static const int N = 100;

	Queue<int, N> mIntQueue;
};
