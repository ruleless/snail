#include "EchoMessage.h"
#include "network/Channel.h"
#include "network/Bundle.h"

void EchoMessage::handle(Channel *pChannel, MemoryStream &s)
{
	printf("serv():%s\n", s.data());
}
