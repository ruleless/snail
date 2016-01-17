#include "EchoMessage.h"
#include "Channel.h"
#include "Bundle.h"
#include "MemoryStream.h"

void EchoMessage::handle(Channel *pChannel, MemoryStream &s)
{
	std::string str;
	s>>str;
	printf("serv():len:%lu data:%s\n", str.length(), str.c_str());
}
