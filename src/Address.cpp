#include "Address.h"

char Address::s_stringBuf[2][32] = {{0}, {0}};

int Address::s_currStringBuf = 0;
const Address Address::NONE(0, 0);

static ObjectPool<Address> s_ObjPool("Address");
ObjectPool<Address>& Address::ObjPool()
{
	return s_ObjPool;
}

void Address::destroyObjPool()
{
// 	DEBUG_MSG(fmt::format("Address::destroyObjPool(): size {}.\n",
// 		s_ObjPool.size()));

	s_ObjPool.destroy();
}

void Address::onReclaimObject()
{
	ip = 0;
	port = 0;
}

Address::Address()
:ip(0)
,port(0)
{
}

Address::Address(uint32 ipArg, uint16 portArg) 
:ip(ipArg)
,port(portArg)
{
}

Address::Address(std::string ipArg, uint16 portArg):
ip(0),
	port(htons(portArg))
{
	u_int32_t addr;
	Address::string2ip(ipArg.c_str(), addr);
	ip = (uint32)addr;
}

int Address::writeToString(char *str, int length) const
{
	uint32	hip = ntohl(ip);
	uint16	hport = ntohs(port);

	return __snprintf(str, length,
		"%d.%d.%d.%d:%d",
		(int)(uchar)(hip>>24),
		(int)(uchar)(hip>>16),
		(int)(uchar)(hip>>8),
		(int)(uchar)(hip),
		(int)hport);
}

char * Address::c_str() const
{
	char *buf = Address::nextStringBuf();
	this->writeToString(buf, 32);
    return buf;
}

const char* Address::ipAsString() const
{
	uint32 hip = ntohl(ip);
	char *buf = Address::nextStringBuf();

	__snprintf(buf, 32, "%d.%d.%d.%d",
		(int)(uchar)(hip>>24),
		(int)(uchar)(hip>>16),
		(int)(uchar)(hip>>8),
		(int)(uchar)(hip));

    return buf;
}

char* Address::nextStringBuf()
{
	s_currStringBuf = (s_currStringBuf + 1) % 2;
	return s_stringBuf[s_currStringBuf];
}

int Address::string2ip(const char *string, u_int32_t &address)
{
	u_int32_t trial;

#ifdef unix
	if (inet_aton(string, (struct in_addr*)&trial) != 0)
#else
	if ((trial = inet_addr(string)) != INADDR_NONE)
#endif
	{
		address = trial;
		return 0;
	}

	struct hostent * hosts = gethostbyname(string);
	if (hosts != NULL)
	{
		address = *(u_int32_t*)(hosts->h_addr_list[0]);
		return 0;
	}

	return -1;
}

int Address::ip2string(u_int32_t address, char * string)
{
	address = ntohl(address);

	int p1, p2, p3, p4;
	p1 = address >> 24;
	p2 = (address & 0xffffff) >> 16;
	p3 = (address & 0xffff) >> 8;
	p4 = (address & 0xff);
	
	return sprintf(string, "%d.%d.%d.%d", p1, p2, p3, p4);
}
