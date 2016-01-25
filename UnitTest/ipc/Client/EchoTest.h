#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "ipc/IpcMessageSlot.h"

class EchoTest : public CppUnit::TestFixture, public IpcMessageHandler
{
	CPPUNIT_TEST_SUITE(EchoTest);
	CPPUNIT_TEST(testEcho);
	CPPUNIT_TEST_SUITE_END();
  public:
    EchoTest();
    virtual ~EchoTest();

	virtual void setUp();

	virtual void tearDown();

	void testEcho();

	virtual void onRecv(IPC_COMP_ID compId, uint8 type, uint8 len, const void *buf);
};
