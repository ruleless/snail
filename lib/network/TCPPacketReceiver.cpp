#include "TCPPacketReceiver.h"
#include "network/Address.h"
#include "network/Bundle.h"
#include "network/Channel.h"
#include "network/EndPoint.h"
#include "network/EventDispatcher.h"
#include "network/NetworkManager.h"
#include "network/EventPoller.h"
#include "network/error_reporter.h"

static ObjectPool<TCPPacketReceiver> s_ObjPool("TCPPacketReceiver");
ObjectPool<TCPPacketReceiver>& TCPPacketReceiver::ObjPool()
{
	return s_ObjPool;
}

void TCPPacketReceiver::destroyObjPool()
{
	DEBUG_MSG(fmt::format("TCPPacketReceiver::destroyObjPool(): size {}.\n", 
		s_ObjPool.size()));

	s_ObjPool.destroy();
}

TCPPacketReceiver::TCPPacketReceiver(EndPoint &endpoint,NetworkManager &networkMgr) 
:PacketReceiver(endpoint, networkMgr)
{
}

TCPPacketReceiver::~TCPPacketReceiver()
{
}

bool TCPPacketReceiver::processRecv(bool expectingPacket)
{
	Channel *pChannel = getChannel();
	Assert(pChannel != NULL);

	if(pChannel->isCondemn())
	{
		return false;
	}

	TCPPacket *pReceiveWindow = TCPPacket::ObjPool().createObject();
	int len = pReceiveWindow->recvFromEndPoint(*mpEndpoint);

	if (len < 0)
	{
		TCPPacket::ObjPool().reclaimObject(pReceiveWindow);

		PacketReceiver::ERecvState recvState = this->checkSocketErrors(len, expectingPacket);

		if(recvState == PacketReceiver::RecvState_Interrupt)
		{
			onGetError(pChannel);
			return false;
		}

		return recvState == PacketReceiver::RecvState_Continue;
	}
	else if(len == 0) // 客户端正常退出
	{
		TCPPacket::ObjPool().reclaimObject(pReceiveWindow);
		onGetError(pChannel);
		return false;
	}
	
	EReason ret = this->processPacket(pChannel, pReceiveWindow);

	if(ret != Reason_Success)
		; // todo
	
	return true;
}

void TCPPacketReceiver::onGetError(Channel* pChannel)
{
	pChannel->condemn();
}

EReason TCPPacketReceiver::processFilteredPacket(Channel *pChannel, Packet *pPacket)
{
	if(pPacket)
	{
		pChannel->addReceiveWindow(pPacket);
	}

	return Reason_Success;
}

PacketReceiver::ERecvState TCPPacketReceiver::checkSocketErrors(int len, bool expectingPacket)
{
#ifdef _WIN32
	DWORD wsaErr = WSAGetLastError();
#endif //def _WIN32

	if (
#ifdef _WIN32
		wsaErr == WSAEWOULDBLOCK && !expectingPacket// send出错大概是缓冲区满了, recv出错已经无数据可读了
#else
		errno == EAGAIN && !expectingPacket			// recv缓冲区已经无数据可读了
#endif
		)
	{
		return RecvState_Break;
	}

#ifdef unix
	if (errno == EAGAIN ||							// 已经无数据可读了
		errno == ECONNREFUSED ||					// 连接被服务器拒绝
		errno == EHOSTUNREACH)						// 目的地址不可到达
	{
		return RecvState_Break;
	}
#else
	/*
	存在的连接被远程主机强制关闭。通常原因为：远程主机上对等方应用程序突然停止运行，或远程主机重新启动，
	或远程主机在远程方套接字上使用了“强制”关闭（参见setsockopt(SO_LINGER)）。
	另外，在一个或多个操作正在进行时，如果连接因“keep-alive”活动检测到一个失败而中断，也可能导致此错误。
	此时，正在进行的操作以错误码WSAENETRESET失败返回，后续操作将失败返回错误码WSAECONNRESET
	*/
	switch(wsaErr)
	{
	case WSAECONNRESET:
		WARNING_MSG("TCPPacketReceiver::processPendingEvents: "
					"Throwing REASON_GENERAL_NETWORK - WSAECONNRESET\n");
		return RecvState_Interrupt;
	case WSAECONNABORTED:
		WARNING_MSG("TCPPacketReceiver::processPendingEvents: "
					"Throwing REASON_GENERAL_NETWORK - WSAECONNABORTED\n");
		return RecvState_Interrupt;
	default:
		break;

	};

#endif // unix

#ifdef _WIN32
	WARNING_MSG(fmt::format("TCPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - {}\n",
				wsaErr));
#else
	WARNING_MSG(fmt::format("TCPPacketReceiver::processPendingEvents: "
				"Throwing REASON_GENERAL_NETWORK - {}\n",
			kbe_strerror()));
#endif

	return RecvState_Continue;
}
