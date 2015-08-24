#include "MessageHandler.h"
#include "network/Channel.h"
#include "network/NetworkManager.h"
#include "network/PacketReceiver.h"

MessageHandler::MessageHandler()
:name("unknown")
,send_size(0)
,send_count(0)
,recv_size(0)
,recv_count(0)
{
}

MessageHandler::~MessageHandler()
{
}

const char* MessageHandler::c_str()
{
	static char buf[MAX_BUF];
	__snprintf(buf, MAX_BUF, "id:%u, len:%d", msgID, msgLen);
	return buf;
}


MessageHandlers::MessageHandlers()
:mMsgHandlers()
{
}

MessageHandlers::~MessageHandlers()
{
	MessageHandlerMap::iterator iter = mMsgHandlers.begin();
	for(; iter != mMsgHandlers.end(); ++iter)
	{
		if(iter->second)
			delete iter->second;
	}
	mMsgHandlers.clear();
}

MessageHandler* MessageHandlers::add(MessageID msgID, MessageHandler* msgHandler)
{
	msgHandler->msgID = msgID;
	msgHandler->onInstall();

	mMsgHandlers[msgHandler->msgID] = msgHandler;

	return msgHandler;
}

MessageHandler* MessageHandlers::find(MessageID msgID)
{
	MessageHandlerMap::iterator iter = mMsgHandlers.find(msgID);
	if(iter != mMsgHandlers.end())
	{
		return iter->second;
	};
	
	return NULL;
}