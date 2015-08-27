EndPoint::EndPoint(u_int32_t networkAddr, u_int16_t networkPort)
#if PLATFORM == PLATFORM_WIN32
		:mSocket(INVALID_SOCKET)
#else
		 :mSocket(-1)
#endif
{
	if(networkAddr)
	{
		mAddress.ip = networkAddr;
		mAddress.port = networkPort;
	}
}

EndPoint::EndPoint(Address address)
#if PLATFORM == PLATFORM_WIN32
		:mSocket(INVALID_SOCKET)
#else
		 :mSocket(-1)
#endif
{
	if(address.ip > 0)
	{
		mAddress = address;
	}
}

INLINE EndPoint::~EndPoint()
{
	this->close();
}

INLINE uint32 EndPoint::getRTT()
{
#if PLATFORM != PLATFORM_WIN32
	struct tcp_info tcpinfo;
	socklen_t len = sizeof(tcpinfo);

	if (getsockopt((*this), SOL_TCP, TCP_INFO, &tcpinfo, &len) != -1)
		return tcpinfo.tcpi_rtt;
#endif

	return 0;
}

INLINE bool EndPoint::good() const
{
#if PLATFORM == PLATFORM_WIN32
	return mSocket != INVALID_SOCKET;
#else
	return mSocket != -1;
#endif
}

INLINE EndPoint::operator SOCKET() const
{
	return mSocket;
}

INLINE SOCKET EndPoint::socket() const
{
	return mSocket;
}

INLINE void EndPoint::setFileDescriptor(int fd)
{
	mSocket = fd;
}

INLINE void EndPoint::socket(int type)
{
	this->setFileDescriptor(::socket(AF_INET, type, 0));
#if PLATFORM == PLATFORM_WIN32
	if ((mSocket == INVALID_SOCKET) && (WSAGetLastError() == WSANOTINITIALISED))
	{
		EndPoint::initNetwork();
		this->setFileDescriptor(::socket(AF_INET, type, 0));
		Assert((mSocket != INVALID_SOCKET) && (WSAGetLastError() != WSANOTINITIALISED) && \
			   "EndPoint::socket: create socket is error!");
	}
#endif
}

INLINE int EndPoint::setnodelay(bool nodelay)
{
	int arg = int(nodelay);
	return setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&arg, sizeof(int));
}

INLINE int EndPoint::setnonblocking(bool nonblocking)
{
#ifdef unix
	int val = nonblocking ? O_NONBLOCK : 0;
	return ::fcntl(mSocket, F_SETFL, val);
#else
	u_long val = nonblocking ? 1 : 0;
	return ::ioctlsocket(mSocket, FIONBIO, &val);
#endif
}

INLINE int EndPoint::setbroadcast(bool broadcast)
{
#ifdef unix
	int val;
	if (broadcast)
	{
		val = 2;
		::setsockopt(mSocket, SOL_IP, IP_MULTICAST_TTL, &val, sizeof(int));
	}
#else
	bool val;
#endif
	val = broadcast ? 1 : 0;
	return ::setsockopt(mSocket, SOL_SOCKET, SO_BROADCAST, (char*)&val, sizeof(val));
}

INLINE int EndPoint::setreuseaddr(bool reuseaddr)
{
#ifdef unix
	int val;
#else
	bool val;
#endif
	val = reuseaddr ? 1 : 0;
	return ::setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR,
						(char*)&val, sizeof(val));
}

INLINE int EndPoint::setlinger(uint16 onoff, uint16 linger)
{
	struct linger l = { 0 };
	l.l_onoff = onoff;
	l.l_linger = linger;
	return setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (const char *) &l, sizeof(l));
}

INLINE int EndPoint::setkeepalive(bool keepalive)
{
#ifdef unix
	int val;
#else
	bool val;
#endif
	val = keepalive ? 1 : 0;
	return ::setsockopt(mSocket, SOL_SOCKET, SO_KEEPALIVE,
						(char*)&val, sizeof(val));
}

INLINE int EndPoint::bind(u_int16_t networkPort, u_int32_t networkAddr)
{
	sockaddr_in	sin;
	memset(&sin, 0, sizeof(sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;
	return ::bind(mSocket, (struct sockaddr *)&sin, sizeof(sin));
}

INLINE int EndPoint::joinMulticastGroup(u_int32_t networkAddr)
{
#ifdef unix
	struct ip_mreqn req;
	req.imr_multiaddr.s_addr = networkAddr;
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;
	return ::setsockopt(mSocket, SOL_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req));
#else
	return -1;
#endif
}

INLINE int EndPoint::quitMulticastGroup(u_int32_t networkAddr)
{
#ifdef unix
	struct ip_mreqn req;
	req.imr_multiaddr.s_addr = networkAddr;
	req.imr_address.s_addr = INADDR_ANY;
	req.imr_ifindex = 0;
	return ::setsockopt(mSocket, SOL_IP, IP_DROP_MEMBERSHIP,&req, sizeof(req));
#else
	return -1;
#endif
}

INLINE int EndPoint::close()
{
	if (mSocket == -1)
	{
		return 0;
	}

#ifdef unix
	int ret = ::close(mSocket);
#else
	int ret = ::closesocket(mSocket);
#endif

	if (ret == 0)
	{
		this->setFileDescriptor(-1);
	}

	return ret;
}

INLINE int EndPoint::getlocaladdress(u_int16_t * networkPort, u_int32_t * networkAddr) const
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
	int ret = ::getsockname(mSocket, (struct sockaddr*)&sin, &sinLen);
	if (ret == 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}
	return ret;
}

INLINE int EndPoint::getremoteaddress(u_int16_t * networkPort, u_int32_t * networkAddr) const
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
	int ret = ::getpeername(mSocket, (struct sockaddr*)&sin, &sinLen);
	if (ret == 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}
	return ret;
}

INLINE const char *EndPoint::c_str() const
{
	return mAddress.c_str();
}

INLINE const Address& EndPoint::addr() const
{
	return mAddress;
}

INLINE void EndPoint::addr(const Address& newAddress)
{
	mAddress = newAddress;
}

INLINE void EndPoint::addr(u_int16_t newNetworkPort, u_int32_t newNetworkAddress)
{
	mAddress.port = newNetworkPort;
	mAddress.ip = newNetworkAddress;
}

INLINE int EndPoint::getremotehostname(std::string * host) const
{
	sockaddr_in		sin;
	socklen_t		sinLen = sizeof(sin);
	int ret = ::getpeername(mSocket, (struct sockaddr*)&sin, &sinLen);
	if (ret == 0)
	{
		hostent* h = gethostbyaddr((char*) &sin.sin_addr,
								   sizeof(sin.sin_addr), AF_INET);

		if (h)
		{
			*host = h->h_name;
		}
		else
		{
			ret = -1;
		}
	}

	return ret;
}

INLINE int EndPoint::sendto(void * gramData, int gramSize, u_int16_t networkPort, u_int32_t networkAddr)
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;

	return this->sendto(gramData, gramSize, sin);
}

INLINE int EndPoint::sendto(void *gramData, int gramSize, struct sockaddr_in &sin)
{
	return ::sendto(mSocket, (char*)gramData, gramSize,	0, (sockaddr*)&sin, sizeof(sin));
}

INLINE int EndPoint::recvfrom(void *gramData, int gramSize, u_int16_t * networkPort, u_int32_t * networkAddr)
{
	sockaddr_in sin;
	int result = this->recvfrom(gramData, gramSize, sin);

	if (result >= 0)
	{
		if (networkPort != NULL) *networkPort = sin.sin_port;
		if (networkAddr != NULL) *networkAddr = sin.sin_addr.s_addr;
	}

	return result;
}

INLINE int EndPoint::recvfrom(void * gramData, int gramSize, struct sockaddr_in & sin)
{
	socklen_t sinLen = sizeof(sin);
	int ret = ::recvfrom(mSocket, (char*)gramData, gramSize, 0, (sockaddr*)&sin, &sinLen);

	return ret;
}

INLINE int EndPoint::listen(int backlog)
{
	return ::listen(mSocket, backlog);
}

INLINE int EndPoint::connect(bool autosetflags)
{
	return connect(mAddress.port, mAddress.ip, autosetflags);
}

INLINE int EndPoint::connect(u_int16_t networkPort, u_int32_t networkAddr, bool autosetflags)
{
	sockaddr_in	sin;
	sin.sin_family = AF_INET;
	sin.sin_port = networkPort;
	sin.sin_addr.s_addr = networkAddr;

	int ret = ::connect(mSocket, (sockaddr*)&sin, sizeof(sin));
	if(autosetflags)
	{
		setnonblocking(true);
		setnodelay(true);
	}
	
	return ret;
}

INLINE EndPoint* EndPoint::accept(u_int16_t *networkPort, u_int32_t *networkAddr, bool autosetflags)
{
	sockaddr_in sin;
	socklen_t sinLen = sizeof(sin);
	int ret = ::accept(mSocket, (sockaddr*)&sin, &sinLen);

#if defined(unix)
	if (ret < 0) return NULL;
#else
	if (ret == INVALID_SOCKET) return NULL;
#endif

	EndPoint *pNew = EndPoint::ObjPool().createObject();

	pNew->setFileDescriptor(ret);
	pNew->addr(sin.sin_port, sin.sin_addr.s_addr);
	
	if(autosetflags)
	{
		pNew->setnonblocking(true);
		pNew->setnodelay(true);
	}
	
	if (networkPort != NULL) 
		*networkPort = sin.sin_port;
	if (networkAddr != NULL) 
		*networkAddr = sin.sin_addr.s_addr;

	return pNew;
}

INLINE int EndPoint::send(const void * gramData, int gramSize)
{
	return ::send(mSocket, (char*)gramData, gramSize, 0);
}

INLINE int EndPoint::recv(void *gramData, int gramSize)
{
	return ::recv(mSocket, (char *)gramData, gramSize, 0);
}


#ifdef unix
INLINE int EndPoint::getInterfaceFlags(char * name, int & flags)
{
	struct ifreq	request;

	strncpy(request.ifr_name, name, IFNAMSIZ);
	if (ioctl(mSocket, SIOCGIFFLAGS, &request) != 0)
	{
		return -1;
	}

	flags = request.ifr_flags;
	return 0;
}

INLINE int EndPoint::getInterfaceAddress(const char * name, u_int32_t & address)
{
	struct ifreq	request;

	strncpy(request.ifr_name, name, IFNAMSIZ);
	if (ioctl(mSocket, SIOCGIFADDR, &request) != 0)
	{
		return -1;
	}

	if (request.ifr_addr.sa_family == AF_INET)
	{
		address = ((sockaddr_in*)&request.ifr_addr)->sin_addr.s_addr;
		return 0;
	}
	else
	{
		return -1;
	}
}

INLINE int EndPoint::getInterfaceNetmask(const char * name,
										 u_int32_t & netmask)
{
	struct ifreq request;
	strncpy(request.ifr_name, name, IFNAMSIZ);

	if (ioctl(mSocket, SIOCGIFNETMASK, &request) != 0)
	{
		return -1;
	}

	netmask = ((sockaddr_in&)request.ifr_netmask).sin_addr.s_addr;

	return 0;
}

#else
INLINE int EndPoint::getInterfaceFlags(char * name, int & flags)
{
	if (!strcmp(name,"eth0"))
	{
		flags = IFF_UP | IFF_BROADCAST | IFF_NOTRAILERS |
				IFF_RUNNING | IFF_MULTICAST;
		return 0;
	}
	else if (!strcmp(name,"lo"))
	{
		flags = IFF_UP | IFF_LOOPBACK | IFF_RUNNING;
		return 0;
	}
	return -1;
}

INLINE int EndPoint::getInterfaceAddress(const char * name, u_int32_t & address)
{
	if (!strcmp(name, "eth0"))
	{
		char myname[256];
		::gethostname(myname, sizeof(myname));

		struct hostent *myhost = gethostbyname(myname);
		if (!myhost)
		{
			return -1;
		}

		address = ((struct in_addr*)(myhost->h_addr_list[0]))->s_addr;
		return 0;
	}
	else if (!strcmp(name, "lo"))
	{
		address = htonl(0x7F000001);
		return 0;
	}

	return -1;
}
#endif
