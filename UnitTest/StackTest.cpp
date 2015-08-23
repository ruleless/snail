#include "StackTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(StackTest);

StackTest::StackTest()
		:mIntStack()
{	
}

StackTest::~StackTest()
{
}

void StackTest::setUp()
{
	mIntStack.clear();
}
	
void StackTest::tearDown()
{
	mIntStack.clear();
}

void StackTest::testOpt()
{
	int loop = 10;
	while (loop--)
	{
		_testOpt();
	}
}

void StackTest::_testOpt()
{
	for (int i = 0; i < N*2; ++i)
	{
		mIntStack.push(i+1);
	}

	CPPUNIT_ASSERT(mIntStack.size() == N);

	int n = 0;
	int curVal = N;
	while (!mIntStack.empty())
	{
		int val = 0;
		mIntStack.pop(val);
		CPPUNIT_ASSERT(val == curVal--);
		++n;
	}

	CPPUNIT_ASSERT(n == N);
	CPPUNIT_ASSERT(mIntStack.empty());
}
