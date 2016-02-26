#include "../EchoMessage.h"
#include "Channel.h"
#include "Bundle.h"

void EchoMessage::handle(Channel *pChannel, MemoryStream &s)
{
	std::string fbuffer;
	s>>fbuffer;

	printf("cli(%s):len:%lu %s", pChannel->addr().ipAsString(), fbuffer.length(), fbuffer.c_str());
	// printf("cli(%s):%s len:%d\n", pChannel->addr().ipAsString(), s.data(), s.length());

	Bundle *pBundle = Bundle::ObjPool().createObject();
	if (pBundle)
	{
		pBundle->newMessage(*this);
		(*pBundle)<<fbuffer;

		pChannel->send(pBundle);
	}
}
