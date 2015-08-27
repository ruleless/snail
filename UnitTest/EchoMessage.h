#ifndef __ECHOMESSAGE_H__
#define __ECHOMESSAGE_H__

#include "network/MessageHandler.h"

class EchoMessage : public MessageHandler
{
  public:
    EchoMessage() : MessageHandler() {}
	virtual ~EchoMessage() {}
	
	virtual void onInstall()
	{
		name = "EchoMessage";
		msgID = 1;
		msgLen = -1;
	}

	virtual void handle(Channel* pChannel, MemoryStream& s);
};

#endif
