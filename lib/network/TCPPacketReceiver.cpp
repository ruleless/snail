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
	else if(len == 0) // �ͻ��������˳�
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
		wsaErr == WSAEWOULDBLOCK && !expectingPacket// send�������ǻ���������, recv�����Ѿ������ݿɶ���
#else
		errno == EAGAIN && !expectingPacket			// recv�������Ѿ������ݿɶ���
#endif
		)
	{
		return RecvState_Break;
	}

#ifdef unix
	if (errno == EAGAIN ||							// �Ѿ������ݿɶ���
		errno == ECONNREFUSED ||					// ���ӱ��������ܾ�
		errno == EHOSTUNREACH)						// Ŀ�ĵ�ַ���ɵ���
	{
		return RecvState_Break;
	}
#else
	/*
	���ڵ����ӱ�Զ������ǿ�ƹرա�ͨ��ԭ��Ϊ��Զ�������϶Եȷ�Ӧ�ó���ͻȻֹͣ���У���Զ����������������
	��Զ��������Զ�̷��׽�����ʹ���ˡ�ǿ�ơ��رգ��μ�setsockopt(SO_LINGER)����
	���⣬��һ�������������ڽ���ʱ�����������keep-alive�����⵽һ��ʧ�ܶ��жϣ�Ҳ���ܵ��´˴���
	��ʱ�����ڽ��еĲ����Դ�����WSAENETRESETʧ�ܷ��أ�����������ʧ�ܷ��ش�����WSAECONNRESET
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
