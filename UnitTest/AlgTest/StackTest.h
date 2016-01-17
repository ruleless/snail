#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "Stack.h"

class StackTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE(StackTest);
	CPPUNIT_TEST(testOpt);
	CPPUNIT_TEST(_testOpt);
	CPPUNIT_TEST_SUITE_END();
  public:
    StackTest();
    virtual ~StackTest();

	virtual void setUp();

	virtual void tearDown();
  protected:
	void testOpt();
	void _testOpt();
  private:
	static const int N = 100;

	Stack<int, N> mIntStack;
};
