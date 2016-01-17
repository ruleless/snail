#include <math.h>
#include "FlexibleArrTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(FlexibleArrTest);

FlexibleArrTest::FlexibleArrTest()
{
}

FlexibleArrTest::~FlexibleArrTest()
{
}

void FlexibleArrTest::setUp()
{
}

void FlexibleArrTest::tearDown()
{
}

void FlexibleArrTest::testRandom()
{
	math::srandom(time(NULL));
	int high = -1 ^ (1<<31);
	for (int i = 10000; i < 100000; ++i)
	{
		int r = math::random(i, high);
		CPPUNIT_ASSERT(r >= i && r <= high);
	}
}

void FlexibleArrTest::visitArray()
{
	FlexibleArray<int> arr;

	for (int i = 0; i < 1000000; ++i)
	{
		int low = 0;
		int high = -1 ^ (1<<31);
		int key = math::random(low, high);
		int val = math::random(low, high);
		arr.set(key, val);

		const int *pval = arr.get(key);

		char msg[256];
		snprintf(msg, sizeof(msg), "i:%d key:%d val:%d", i, key, val);
		CPPUNIT_ASSERT_MESSAGE(msg, pval);
		CPPUNIT_ASSERT_MESSAGE(msg, *pval == val);
	}
}
