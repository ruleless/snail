#include "MessageHandler.h"
// #include "common/md5.h"
#include "network/Channel.h"
#include "network/NetworkManager.h"
#include "network/PacketReceiver.h"
#include "network/fixed_messages.h"

MessageHandlers* MessageHandlers::pMainMessageHandlers = 0;
std::vector<MessageHandlers*>* g_pMessageHandlers;

static FixedMessages* g_fm;

//-------------------------------------------------------------------------------------
MessageHandlers::MessageHandlers():
msgHandlers_(),
msgID_(1),
exposedMessages_()
{
	g_fm = FixedMessages::getSingletonPtr();
	if(g_fm == NULL)
		g_fm = new FixedMessages;

	FixedMessages::getSingleton().loadConfig("server/messages_fixed.xml");
	messageHandlers().push_back(this);
}

//-------------------------------------------------------------------------------------
MessageHandlers::~MessageHandlers()
{
	MessageHandlerMap::iterator iter = msgHandlers_.begin();
	for(; iter != msgHandlers_.end(); ++iter)
	{
		if(iter->second)
			delete iter->second;
	};
}

//-------------------------------------------------------------------------------------
MessageHandler::MessageHandler():
pArgs(NULL),
pMessageHandlers(NULL),
send_size(0),
send_count(0),
recv_size(0),
recv_count(0)
{
}

//-------------------------------------------------------------------------------------
MessageHandler::~MessageHandler()
{
	SafeDelete(pArgs);
}

//-------------------------------------------------------------------------------------
const char* MessageHandler::c_str()
{
	static char buf[MAX_BUF];
	__snprintf(buf, MAX_BUF, "id:%u, len:%d", msgID, msgLen);
	return buf;
}

//-------------------------------------------------------------------------------------
MessageHandler* MessageHandlers::add(std::string ihName, MessageArgs* args, 
	int32 msgLen, MessageHandler* msgHandler)
{
	if(msgID_ == 1)
	{
		//printf("\n------------------------------------------------------------------\n");
		//printf("KBEMessage_handlers begin:\n");
	}
	
	//bool isfixedMsg = false;

	FixedMessages::MSGInfo* msgInfo = FixedMessages::getSingleton().isFixed(ihName.c_str());
	if(msgInfo == NULL)
	{
		while(true)
		{
			if(FixedMessages::getSingleton().isFixed(msgID_))
			{
				msgID_++;
				//isfixedMsg = true;
			}
			else
			{
				break;
			}
		};

		msgHandler->msgID = msgID_++;
	}
	else
	{
		msgHandler->msgID = msgInfo->msgid;
	}
	
	msgHandler->name = ihName;					
	msgHandler->pArgs = args;
	msgHandler->msgLen = msgLen;	
	msgHandler->exposed = false;
	msgHandler->pMessageHandlers = this;
	msgHandler->onInstall();

	msgHandlers_[msgHandler->msgID] = msgHandler;
	
	if(msgLen == NETWORK_VARIABLE_MESSAGE)
	{
		//printf("\tMessageHandlers::add(%d): name=%s, msgID=%d, size=Variable.\n", 
		//	(int32)msgHandlers_.size(), ihName.c_str(), msgHandler->msgID);
	}
	else
	{
		if(msgLen == 0)
		{
			msgHandler->msgLen = args->dataSize();

			if(msgHandler->type() == NETWORK_MESSAGE_TYPE_ENTITY)
			{
				msgHandler->msgLen += sizeof(int32);
			}
		}
		
		//printf("\tMessageHandlers::add(%d): name=%s, msgID=%d, size=Fixed(%d).\n", 
		//		(int32)msgHandlers_.size(), ihName.c_str(), msgHandler->msgID, msgHandler->msgLen);
	}

	//if(isfixedMsg)
	//	printf("\t\t!!!message is fixed.!!!\n");

	return msgHandler;
}

//-------------------------------------------------------------------------------------
MessageHandler* MessageHandlers::find(MessageID msgID)
{
	MessageHandlerMap::iterator iter = msgHandlers_.find(msgID);
	if(iter != msgHandlers_.end())
	{
		return iter->second;
	};
	
	return NULL;
}

//-------------------------------------------------------------------------------------
std::vector<MessageHandlers*>& MessageHandlers::messageHandlers()
{
	if(g_pMessageHandlers == NULL)
		g_pMessageHandlers = new std::vector<MessageHandlers *>;

	return *g_pMessageHandlers;
}

//-------------------------------------------------------------------------------------
void MessageHandlers::finalise(void)
{
	SafeDelete(g_fm);
	SafeDelete(g_pMessageHandlers);
}

//-------------------------------------------------------------------------------------
bool MessageHandlers::pushExposedMessage(std::string msgname)
{
	exposedMessages_.push_back(msgname);
	return true;
}
