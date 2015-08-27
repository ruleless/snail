#ifndef __MESSAGEHANDLER_H__
#define __MESSAGEHANDLER_H__

#include "common/MemoryStream.h"
#include "network/NetworkDef.h"

class Channel;
class MessageHandler
{
  public:
	std::string name;
	MessageID msgID;
	int32 msgLen; // 如果长度为-1则为变长消息

	// 统计数据
	volatile mutable uint32 send_size;
	volatile mutable uint32 send_count;
	volatile mutable uint32 recv_size;
	volatile mutable uint32 recv_count;
  public:
	MessageHandler();
	virtual ~MessageHandler();

	const char* c_str();

	uint32 sendsize() const { return send_size; }
	uint32 sendcount() const { return send_count; }
	uint32 sendavgsize() const { return (send_count <= 0) ? 0 : send_size / send_count; }

	uint32 recvsize() const { return recv_size; }
	uint32 recvcount() const { return recv_count; }
	uint32 recvavgsize() const { return (recv_count <= 0) ? 0 : recv_size / recv_count; }

	virtual int32 msglenMax() { return NETWORK_MESSAGE_MAX_SIZE; }

	virtual void onInstall() {}

	virtual void handle(Channel* pChannel, MemoryStream& s) = 0;
};

class MessageHandlers
{
  public:
	typedef std::map<MessageID, MessageHandler *> MessageHandlerMap;

	MessageHandlers();
	~MessageHandlers();
	
	MessageHandler* add(MessageID msgID, MessageHandler* msgHandler);
	MessageHandler* find(MessageID msgID);

	void remove(MessageID msgID);
  private:
	MessageHandlerMap mMsgHandlers;
};

#endif // __MESSAGEHANDLER_H__
