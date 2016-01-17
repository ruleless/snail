#include "cppunit/extensions/TestFactoryRegistry.h"
#include "cppunit/ui/text/TestRunner.h"

#include "ClientTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ClientTest);

ClientTest::ClientTest()
		:mpDispatcher(NULL)
		,mpNetMgr(NULL)
		,mpEndPt(NULL)
		,mpChannel(NULL)
{
}

ClientTest::~ClientTest()
{
}
	
void ClientTest::setUp()
{
	mMsgHandlers.add(1, &mEchoMessage);
	
	mpDispatcher = new EventDispatcher();
	mpNetMgr = new NetworkManager(mpDispatcher);

	mpDispatcher->addTask(this);

	// 标准输入
#if PLATFORM == PLATFORM_UNIX
	mpDispatcher->registerReadFileDescriptor(STDIN_FILENO, this);
#endif

	// 建立网络连接
	Address addr("127.0.0.1", 60000);
	mpEndPt = EndPoint::ObjPool().createObject();
	mpEndPt->socket(SOCK_STREAM);
	mpEndPt->addr(addr);
	CPPUNIT_ASSERT_MESSAGE("connect", mpEndPt->connect() == 0);

	mpChannel = Channel::ObjPool().createObject();
	CPPUNIT_ASSERT_MESSAGE("init channel", mpChannel->initialize(*mpNetMgr, mpEndPt, Channel::External));

	CPPUNIT_ASSERT_MESSAGE("reg channel", mpNetMgr->registerChannel(mpChannel));
}
	
void ClientTest::tearDown()
{	
	mMsgHandlers.remove(1);

	mpDispatcher->cancelTask(this);
#if PLATFORM == PLATFORM_UNIX
	mpDispatcher->deregisterReadFileDescriptor(STDIN_FILENO);
#endif

	SafeDelete(mpNetMgr);
	SafeDelete(mpDispatcher);
}

bool ClientTest::process()
{
	mpNetMgr->processChannels(&mMsgHandlers);
	return true;
}

int ClientTest::handleInputNotification(int fd)
{
	char buff[MAX_BUF];
	memset(buff, 0, sizeof(buff));

	int n = 0;
#if PLATFORM == PLATFORM_UNIX
	n = read(STDIN_FILENO, buff, MAX_BUF);
#endif
	if (n <= 0)
		return 0;

	Bundle *pBundle = Bundle::ObjPool().createObject();
	pBundle->newMessage(mEchoMessage);
	(*pBundle)<<buff;

	CPPUNIT_ASSERT(mpChannel);
	mpChannel->send(pBundle);
	return 0;
}

void ClientTest::runEcho()
{
	CPPUNIT_ASSERT(mpDispatcher && mpNetMgr);
	mpDispatcher->processUntilBreak();
}
