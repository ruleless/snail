#ifndef __NETWORKDEF_H__
#define __NETWORKDEF_H__

class Channel;
class MessageHandler;

class InputNotificationHandler
{
public:
	virtual ~InputNotificationHandler() {};
	virtual int handleInputNotification(int fd) = 0;
};

class OutputNotificationHandler
{
public:
	virtual ~OutputNotificationHandler() {};
	virtual int handleOutputNotification(int fd) = 0;
};

class ChannelTimeOutHandler
{
public:
	virtual void onChannelTimeOut(Channel * pChannel) = 0;
};

class ChannelDeregisterHandler
{
public:
	virtual void onChannelDeregister(Channel * pChannel) = 0;
};

class NetworkStatsHandler
{
public:
	virtual void onSendMessage(const MessageHandler & msgHandler, int size) = 0;
	virtual void onRecvMessage(const MessageHandler & msgHandler, int size) = 0;
};

#endif // __NETWORKDEF_H__
