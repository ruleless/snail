#include "QueueTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(QueueTest);

QueueTest::QueueTest()
		:mIntQueue()
{
}

QueueTest::~QueueTest()
{
}

void QueueTest::setUp()
{
	mIntQueue.clear();
}

void QueueTest::tearDown()
{
	mIntQueue.clear();
}

void QueueTest::testQueueOperation()
{
	int loop = 10;
	while (loop--)
	{
		_testQueueOperation();
	}
}

void QueueTest::_testQueueOperation()
{
	for (int i = 0; i < N+N; ++i)
	{
		mIntQueue.pushBack(i+1);
	}
	CPPUNIT_ASSERT(mIntQueue.size() == N);

	int curVal = 1;
	int n = 0;
	while (!mIntQueue.empty())
	{
		int val = 0;
		mIntQueue.popFront(val);

		CPPUNIT_ASSERT(val == curVal++);
		++n;
	}

	CPPUNIT_ASSERT(n == N);
	CPPUNIT_ASSERT(mIntQueue.size() == 0);
}
