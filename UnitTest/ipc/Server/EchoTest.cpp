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
	createTrace();
	output2Console();

	CPPUNIT_ASSERT(_gIpcMsgSlot.open());
	_gIpcMsgSlot.subscribe(1, this);
}

void EchoTest::tearDown()
{
	_gIpcMsgSlot.unsubscribe(1);
	_gIpcMsgSlot.close();

	closeTrace();
}

void EchoTest::testEcho()
{
	while (1)
	{
		_gIpcMsgSlot.process();
	}
}

void EchoTest::onRecv(IPC_COMP_ID compId, uint8 type, uint8 len, const void *buf)
{
	ostrbuf ob;
	ob<<"recv from "<<compId<<"  message type="<<type;
	ob.push_back(buf, len);
	printf("%s\n", ob.c_str());

	_gIpcMsgSlot.postMessage(compId, type, len, buf);
}
