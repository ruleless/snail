#include "EchoMessage.h"
#include "network/Channel.h"
#include "network/Bundle.h"
#include "common/MemoryStream.h"

void EchoMessage::handle(Channel *pChannel, MemoryStream &s)
{
	std::string str;
	s>>str;
	printf("serv():len:%lu data:%s\n", str.length(), (char *)str.c_str());
}
