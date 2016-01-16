#ifndef __ENDPOINT_H__
#define __ENDPOINT_H__

#include "common.h"
#include "ObjectPool.h"
#include "Address.h"
#include "NetworkDef.h"

class Bundle;
class EndPoint : public PoolObject
{
  public:
	static ObjectPool<EndPoint>& ObjPool();
	static void destroyObjPool();
	virtual void onReclaimObject();
	virtual size_t getPoolObjectBytes()
	{
		size_t bytes = sizeof(SOCKET) + mAddress.getPoolObjectBytes();
		return bytes;
	}

	EndPoint(Address address);
	EndPoint(u_int32_t networkAddr = 0, u_int16_t networkPort = 0);
	virtual ~EndPoint();

	operator SOCKET() const;
	
	static void initNetwork();
	bool good() const;

	void socket(int type);
	SOCKET socket() const;
	
	INLINE void setFileDescriptor(int fd);

	INLINE int joinMulticastGroup(u_int32_t networkAddr);
	INLINE int quitMulticastGroup(u_int32_t networkAddr);
	
	INLINE int close();
	
	INLINE int setnonblocking(bool nonblocking);
	INLINE int setbroadcast(bool broadcast);
	INLINE int setreuseaddr(bool reuseaddr);
	INLINE int setkeepalive(bool keepalive);
	INLINE int setnodelay(bool nodelay = true);
	INLINE int setlinger(uint16 onoff, uint16 linger);

	INLINE int bind(u_int16_t networkPort = 0, u_int32_t networkAddr = INADDR_ANY);

	int listen(int backlog = 5);

	int connect(u_int16_t networkPort, u_int32_t networkAddr = INADDR_BROADCAST, bool autosetflags = true);
	int connect(bool autosetflags = true);

	EndPoint* accept(u_int16_t *networkPort = NULL, u_int32_t *networkAddr = NULL, bool autosetflags = true);
	
	INLINE int send(const void *gramData, int gramSize);
	void send(Bundle *pBundle);
	void sendto(Bundle *pBundle, u_int16_t networkPort, u_int32_t networkAddr = BROADCAST);

	int recv(void *gramData, int gramSize);
	bool recvAll(void *gramData, int gramSize);
	
	INLINE uint32 getRTT();

	int getInterfaceFlags(char *name, int &flags);
	int getInterfaceAddress(const char *name, u_int32_t &address);
	int getInterfaceNetmask(const char *name, u_int32_t &netmask);
	bool getInterfaces(std::map<u_int32_t, std::string> &interfaces);

	int findIndicatedInterface(const char *spec, u_int32_t &address);
	int findDefaultInterface(char *name);

	int getInterfaceAddressByName(const char *name, u_int32_t &address);
	int getInterfaceAddressByMAC(const char *mac, u_int32_t &address);
	int getDefaultInterfaceAddress(u_int32_t &address);

	int getBufferSize(int optname) const;
	bool setBufferSize(int optname, int size);
	
	int getlocaladdress(u_int16_t *networkPort, u_int32_t *networkAddr) const;
	int getremoteaddress(u_int16_t *networkPort, u_int32_t *networkAddr) const;
	
	Address getLocalAddress() const;
	Address getRemoteAddress() const;

	bool getClosedPort(Address &closedPort);

	const char *c_str() const;
	int getremotehostname(std::string *name) const;
	
	int sendto(void *gramData, int gramSize, u_int16_t networkPort, u_int32_t networkAddr = BROADCAST);
	INLINE int sendto(void *gramData, int gramSize, struct sockaddr_in &sin);
	INLINE int recvfrom(void *gramData, int gramSize, u_int16_t *networkPort, u_int32_t *networkAddr);
	INLINE int recvfrom(void *gramData, int gramSize, struct sockaddr_in &sin);
	
	INLINE const Address& addr() const;
	void addr(const Address& newAddress);
	void addr(u_int16_t newNetworkPort, u_int32_t newNetworkAddress);

	bool waitSend();
  protected:
	SOCKET mSocket;
	Address mAddress;
};

#ifdef _INLINE
#include "EndPoint.inl"
#endif
#endif
