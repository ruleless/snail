#ifndef __ADDRESS_H__
#define __ADDRESS_H__

#include "common.h"
#include "ObjectPool.h"
#include "NetworkDef.h"

class Address : public PoolObject
{
  public:
	static const Address NONE;

	static ObjectPool<Address>& ObjPool();
	static void destroyObjPool();
	virtual void onReclaimObject();

	virtual size_t getPoolObjectBytes()
	{
		size_t bytes = sizeof(ip) + sizeof(port);

		return bytes;
	}

	Address();
	Address(uint32 ipArg, uint16 portArg);
	Address(std::string ipArg, uint16 portArg);
	
	virtual ~Address(){}

	int writeToString(char *str, int length) const;

	operator char*() const { return this->c_str(); }

	char* c_str() const;
	const char* ipAsString() const;
	bool isNone() const	{ return this->ip == 0; }

	static int string2ip(const char *string, u_int32_t &address);
	static int ip2string(u_int32_t address, char *string);
  public:
	uint32 ip;
	uint16 port;
  private:
	static char s_stringBuf[2][32];
	static int s_currStringBuf;
	static char *nextStringBuf();
}; 

inline bool operator==(const Address &a, const Address &b)
{
	return (a.ip == b.ip) && (a.port == b.port);
}

inline bool operator!=(const Address &a, const Address &b)
{
	return (a.ip != b.ip) || (a.port != b.port);
}

inline bool operator<(const Address &a, const Address &b)
{
	return (a.ip < b.ip) || (a.ip == b.ip && (a.port < b.port));
}

#endif // __ADDRESS_H__
