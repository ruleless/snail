#include "EchoTest.h"
#include "StrBuffer.h"
#include "Trace.h"
#include "ipc/IpcMessageSlot.h"

CPPUNIT_TEST_SUITE_REGISTRATION(EchoTest);

IpcMessageSlot _gIpcMsgSlot("/EchoTest", IpcComp_Master);

EchoTest::EchoTest()
{
}

EchoTest::~EchoTest()
{
}

void EchoTest::setUp()
{
	CPPUNIT_ASSERT(_gIpcMsgSlot.open());
	_gIpcMsgSlot.subscribe(1, this);
}

void EchoTest::tearDown()
{
	_gIpcMsgSlot.unsubscribe(1);
	_gIpcMsgSlot.close();
}

void EchoTest::testEcho()
{
	fd_set fdSet;
	int maxFd;
	struct timeval tm;
	FD_SET(STDIN_FILENO, &fdSet);
	maxFd = STDIN_FILENO+1;
	tm.tv_sec = 0;
	tm.tv_usec = 16000;

	bool bLoop = true;
	while (bLoop)
	{
		int nready = select(maxFd, &fdSet, NULL, NULL, &tm);
		tm.tv_sec = 0;
		tm.tv_usec = 16000;
		if (nready > 0 && FD_ISSET(STDIN_FILENO, &fdSet))
		{
			char buf[1024];
			int n = read(STDIN_FILENO, buf, 1024);
			if (n > 0)
			{
				switch(buf[0])
				{
				case 'q':
					TraceLn("quit!");
					bLoop = false;
					break;
				}
			}
		}

		FD_SET(STDIN_FILENO, &fdSet);

		_gIpcMsgSlot.process();
	}
}

void EchoTest::onRecv(IPC_COMP_ID compId, uint8 type, uint8 len, const void *buf)
{
	ostrbuf ob;
	ob<<"recvfrom:"<<compId<<" messagetype="<<type<<" len="<<len;
	if (len > 0)
	{
		ob<<" msg="<<(const char *)buf;
	}
	printf("%s\n", ob.c_str());

	_gIpcMsgSlot.postMessage(compId, type, len, buf);
}
