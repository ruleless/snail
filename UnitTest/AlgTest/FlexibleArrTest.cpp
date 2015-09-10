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

void FlexibleArrTest::visitArray()
{
	FlexibleArray<int> arr;
	for (int i = 1000; i < 10000; ++i)
	{
		arr.set(i, i);
		const int *pval = arr.get(i);
		CPPUNIT_ASSERT(pval);
		CPPUNIT_ASSERT(*pval == i);
	}
}
