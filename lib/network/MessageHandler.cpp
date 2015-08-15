#include "MessageHandler.h"
#include "common/md5.h"
#include "network/Channel.h"
#include "network/NetworkManager.h"
#include "network/PacketReceiver.h"
#include "network/fixed_messages.h"
#include "helper/watcher.h"
#include "xml/xml.h"
#include "resmgr/resmgr.h"	

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
	kbe_snprintf(buf, MAX_BUF, "id:%u, len:%d", msgID, msgLen);
	return buf;
}

//-------------------------------------------------------------------------------------
bool MessageHandlers::initializeWatcher()
{
	std::vector< std::string >::iterator siter = exposedMessages_.begin();
	for(; siter != exposedMessages_.end(); ++siter)
	{
		MessageHandlerMap::iterator iter = msgHandlers_.begin();
		for(; iter != msgHandlers_.end(); ++iter)
		{
			if((*siter) == iter->second->name)
			{
				iter->second->exposed = true;
			}
		}
	}

	MessageHandlerMap::iterator iter = msgHandlers_.begin();
	for(; iter != msgHandlers_.end(); ++iter)
	{
		char buf[MAX_BUF];
		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/id", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second->msgID);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/len", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second->msgLen);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/sentSize", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::sendsize);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/sentCount", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::sendcount);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/sentAvgSize", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::sendavgsize);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/recvSize", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::recvsize);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/recvCount", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::recvsize);

		kbe_snprintf(buf, MAX_BUF, "network/messages/%s/recvAvgSize", iter->second->name.c_str());
		WATCH_OBJECT(buf, iter->second, &MessageHandler::recvavgsize);
	}

	return true;
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
				msgHandler->msgLen += sizeof(ENTITY_ID);
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
std::string MessageHandlers::getDigestStr()
{
	static KBE_MD5 md5;

	if(!md5.isFinal())
	{
		std::map<uint16, std::pair< std::string, std::string> > errsDescrs;

		TiXmlNode *rootNode = NULL;
		SmartPointer<XML> xml(new XML(Resmgr::getSingleton().matchRes("server/server_errors.xml").c_str()));

		if(!xml->isGood())
		{
			ERROR_MSG(fmt::format("MessageHandlers::getDigestStr(): load {} is failed!\n",
				Resmgr::getSingleton().matchRes("server/server_errors.xml")));

			return "";
		}

		int32 isize = 0;

		rootNode = xml->getRootNode();
		if(rootNode == NULL)
		{
			// root节点下没有子节点了
			return "";
		}

		XML_FOR_BEGIN(rootNode)
		{
			TiXmlNode* node = xml->enterNode(rootNode->FirstChild(), "id");
			TiXmlNode* node1 = xml->enterNode(rootNode->FirstChild(), "descr");

			int32 val1 = xml->getValInt(node);
			md5.append((void*)&val1, sizeof(int32));

			std::string val2 = xml->getKey(rootNode);
			md5.append((void*)val2.c_str(), val2.size());

			std::string val3 = xml->getVal(node1);
			md5.append((void*)val3.c_str(), val3.size());
			isize++;
		}
		XML_FOR_END(rootNode);

		md5.append((void*)&isize, sizeof(int32));

		

		std::vector<MessageHandlers*>& msgHandlers = messageHandlers();
		isize += msgHandlers.size();
		md5.append((void*)&isize, sizeof(int32));

		std::vector<MessageHandlers*>::const_iterator rootiter = msgHandlers.begin();
		for(; rootiter != msgHandlers.end(); ++rootiter)
		{
			isize += (*rootiter)->msgHandlers().size();
			md5.append((void*)&isize, sizeof(int32));

			MessageHandlerMap::const_iterator iter = (*rootiter)->msgHandlers().begin();
			for(; iter != (*rootiter)->msgHandlers().end(); ++iter)
			{
				MessageHandler* pMessageHandler = iter->second;
			
				md5.append((void*)pMessageHandler->name.c_str(), pMessageHandler->name.size());
				md5.append((void*)&pMessageHandler->msgID, sizeof(MessageID));
				md5.append((void*)&pMessageHandler->msgLen, sizeof(int32));
				md5.append((void*)&pMessageHandler->exposed, sizeof(bool));
	 
				int32 argsize = pMessageHandler->pArgs->strArgsTypes.size();
				md5.append((void*)&argsize, sizeof(int32));

				int32 argsdataSize = pMessageHandler->pArgs->dataSize();
				md5.append((void*)&argsdataSize, sizeof(int32));

				int32 argstype = (int32)pMessageHandler->pArgs->type();
				md5.append((void*)&argstype, sizeof(int32));

				std::vector<std::string>::iterator saiter = pMessageHandler->pArgs->strArgsTypes.begin();
				for(; saiter != pMessageHandler->pArgs->strArgsTypes.end(); ++saiter)
				{
					md5.append((void*)(*saiter).c_str(), (*saiter).size());
				}
			}
		}
	}

	return md5.getDigestStr();
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
