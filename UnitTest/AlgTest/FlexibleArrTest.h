#ifndef __FLEXIBLE_ARR_TEST_H__
#define __FLEXIBLE_ARR_TEST_H__

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include "algorithm/FlexibleArray.h"

class FlexibleArrTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(FlexibleArrTest);
	CPPUNIT_TEST(testRandom);
	CPPUNIT_TEST(visitArray);	
	CPPUNIT_TEST_SUITE_END();
  public:
    FlexibleArrTest();
    virtual ~FlexibleArrTest();
	
	virtual void setUp();
	
	virtual void tearDown();
  protected:
	void testRandom();
	void visitArray();
};

#endif
