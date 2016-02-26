#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include "common.h"
#include "Task.h"
#include "EventDispatcher.h"
#include "NetworkManager.h"
#include "EndPoint.h"
#include "Channel.h"

#include "../EchoMessage.h"
#include "MessageHandler.h"

class ClientTest : public CppUnit::TestFixture, public Task, public InputNotificationHandler
{
	CPPUNIT_TEST_SUITE(ClientTest);
	CPPUNIT_TEST(runEcho);
	CPPUNIT_TEST_SUITE_END();
  public:
    ClientTest();
    virtual ~ClientTest();

	virtual void setUp();

	virtual void tearDown();

	virtual bool process();

	virtual int handleInputNotification(int fd);

	void runEcho();
  private:
	EventDispatcher *mpDispatcher;
	NetworkManager *mpNetMgr;

	EndPoint *mpEndPt;
	Channel *mpChannel;

	EchoMessage mEchoMessage;
	MessageHandlers mMsgHandlers;
};
