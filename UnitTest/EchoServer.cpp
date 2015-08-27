#include "EchoMessage.h"
#include "network/Channel.h"
#include "network/Bundle.h"

void EchoMessage::handle(Channel *pChannel, MemoryStream &s)
{
	char buf[1024];
	std::string fbuffer;

	__snprintf(buf, 1024, "wpos: %lu, rpos=%lu.\n", (unsigned long)s.wpos(), (unsigned long)s.rpos());
	fbuffer += buf;

	for(uint32 i = s.rpos(); i < s.wpos(); ++i)
	{
		__snprintf(buf, 1024, "%c", s.read<uint8>(i));
		fbuffer += buf;
	}

	printf("cli(%s):%s\n", pChannel->addr().ipAsString(), fbuffer.c_str());
	// printf("cli(%s):%s len:%d\n", pChannel->addr().ipAsString(), s.data(), s.length());

	Bundle *pBundle = Bundle::ObjPool().createObject();
	if (pBundle)
	{
		pBundle->newMessage(*this);
		pBundle->append(s);

		pChannel->send(pBundle);
	}
}
