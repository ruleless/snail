#ifndef __SERVERTEST_H__
#define __SERVERTEST_H__

#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include "common.h"
#include "Task.h"
#include "NetworkManager.h"
#include "EventDispatcher.h"

#include "../EchoMessage.h"
#include "MessageHandler.h"

class ServerTest : public CppUnit::TestFixture, public Task
{
	CPPUNIT_TEST_SUITE(ServerTest);
	CPPUNIT_TEST(runServer);
	CPPUNIT_TEST_SUITE_END();	
  public:
    ServerTest();
    virtual ~ServerTest();
	
	virtual void setUp();
	
	virtual void tearDown();

	virtual bool process();

	void runServer();
  private:
	EventDispatcher *mpDispatcher;
	NetworkManager *mpNetMgr;

	EchoMessage mEchoMessage;
	MessageHandlers mMsgHandlers;
};

#endif
