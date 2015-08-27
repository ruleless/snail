#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include "common/common.h"
#include "common/Task.h"
#include "network/EventDispatcher.h"
#include "network/NetworkManager.h"
#include "network/EndPoint.h"
#include "network/Channel.h"

#include "EchoMessage.h"
#include "network/MessageHandler.h"

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
