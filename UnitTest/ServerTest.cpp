#include "ServerTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ServerTest);

ServerTest::ServerTest()
		:mpDispatcher(NULL)
		,mpNetMgr(NULL)
{
}

ServerTest::~ServerTest()
{
}

void ServerTest::setUp()
{
	mMsgHandlers.add(1, &mEchoMessage);
	
	mpDispatcher = new EventDispatcher();
	mpNetMgr = new NetworkManager(mpDispatcher, 60000, 60000);
	
	mpDispatcher->addTask(this);
}
	
void ServerTest::tearDown()
{
	mMsgHandlers.remove(1);
	
	mpDispatcher->cancelTask(this);
	
	SafeDelete(mpNetMgr);
	SafeDelete(mpDispatcher);	
}

bool ServerTest::process()
{
	mpNetMgr->processChannels(&mMsgHandlers);
	return true;
}

void ServerTest::runServer()
{
	CPPUNIT_ASSERT(mpDispatcher && mpNetMgr);

	mpDispatcher->processUntilBreak();
}
