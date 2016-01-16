#include "Listener.h"
#include "NetworkManager.h"
#include "PacketReceiver.h"
#include "Channel.h"

Listener::Listener(EndPoint &endpoint, Channel::ETraits traits, NetworkManager &networkMgr)
		:mNetworkManager(networkMgr)
		,mEndpoint(endpoint)
		,mTraits(traits)		
{
}

Listener::~Listener()
{
}

int Listener::handleInputNotification(int fd)
{
	int newConnections = 0;

	while(newConnections++ < 256)
	{
		EndPoint *pNewEndPoint = mEndpoint.accept();

		if(pNewEndPoint == NULL)
		{
			if(newConnections == 1)
			{
				// 				WARNING_MSG(fmt::format("PacketReceiver::handleInputNotification: accept endpoint({}) {}!\n",
				// 					 fd, __strerror()));
			}

			break;
		}
		else
		{
			Channel *pChannel = Channel::ObjPool().createObject();
			bool ret = pChannel->initialize(mNetworkManager, pNewEndPoint, mTraits);
			if(!ret)
			{
				// 				ERROR_MSG(fmt::format("Listener::handleInputNotification: initialize({}) is failed!\n",
				// 					pChannel->c_str()));

				pChannel->destroy();
				Channel::ObjPool().reclaimObject(pChannel);
				return 0;
			}

			if(!mNetworkManager.registerChannel(pChannel))
			{
				// 				ERROR_MSG(fmt::format("Listener::handleInputNotification: registerChannel({}) is failed!\n",
				// 					pChannel->c_str()));

				pChannel->destroy();
				Channel::ObjPool().reclaimObject(pChannel);
			}
		}
	}

	return 0;
}
