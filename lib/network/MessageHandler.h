#ifndef KBE_MESSAGE_HANDLER_H
#define KBE_MESSAGE_HANDLER_H

#include "common/memorystream.h"
#include "common/smartpointer.h"
#include "helper/debug_helper.h"
#include "network/NetworkDef.h"

class KBE_MD5;
class Channel;
class MessageHandlers;

class MessageArgs
{
public:
	enum EMessageArgsType
	{
		MessageArgsType_Variable = -1,
		MessageArgsType_Fixed = 0,
	};

	MessageArgs() : strArgsTypes() {}
	virtual ~MessageArgs() {};
	virtual void createFromStream(MemoryStream& s) = 0;
	virtual void addToStream(MemoryStream& s) = 0;
	virtual int32 dataSize(void) = 0;
	virtual MessageArgs::EMessageArgsType type(void) { return MessageArgsType_Fixed; }

	std::vector<std::string> strArgsTypes;
};

struct ExposedMessageInfo
{
	std::string name;
	MessageID id;
	int16 msgLen; // 对外消息不会超过1500
	int8 argsType;
	std::vector<uint8> argsTypes;
};

class MessageHandler
{
public:
	MessageHandler();
	virtual ~MessageHandler();

	std::string name;
	MessageID msgID;
	MessageArgs* pArgs;
	int32 msgLen;					// 如果长度为-1则为非固定长度消息
	bool exposed;
	MessageHandlers* pMessageHandlers;

	// stats
	volatile mutable uint32 send_size;
	volatile mutable uint32 send_count;
	volatile mutable uint32 recv_size;
	volatile mutable uint32 recv_count;

	uint32 sendsize() const  { return send_size; }
	uint32 sendcount() const  { return send_count; }
	uint32 sendavgsize() const  { return (send_count <= 0) ? 0 : send_size / send_count; }

	uint32 recvsize() const  { return recv_size; }
	uint32 recvcount() const  { return recv_count; }
	uint32 recvavgsize() const  { return (recv_count <= 0) ? 0 : recv_size / recv_count; }

	/**
		默认返回类别为组件消息
	*/
	virtual NETWORK_MESSAGE_TYPE type() const
	{ 
		return NETWORK_MESSAGE_TYPE_COMPONENT; 
	}

	virtual int32 msglenMax(){ return NETWORK_MESSAGE_MAX_SIZE; }

	const char* c_str();

	/**
		当这个handler被正是安装到MessageHandlers后被调用
	*/
	virtual void onInstall(){}

	virtual void handle(Channel* pChannel, MemoryStream& s)
	{
		pArgs->createFromStream(s);
		
		// 将参数传给最终的接口
	};
};

class MessageHandlers
{
public:
	static MessageHandlers* pMainMessageHandlers;
	typedef std::map<MessageID, MessageHandler *> MessageHandlerMap;
	MessageHandlers();
	~MessageHandlers();
	
	MessageHandler* add(std::string ihName, MessageArgs* args, int32 msgLen, 
						MessageHandler* msgHandler);
	
	bool pushExposedMessage(std::string msgname);

	MessageHandler* find(MessageID msgID);
	
	MessageID lastMsgID() {return msgID_ - 1;}

	bool initializeWatcher();
	
	static void finalise(void);
	static std::vector<MessageHandlers*>& messageHandlers();

	const MessageHandlerMap& msgHandlers(){ return msgHandlers_; }

	static std::string getDigestStr();

private:
	MessageHandlerMap msgHandlers_;
	MessageID msgID_;

	std::vector< std::string > exposedMessages_;
};

#endif 
