#include "Listener.h"
#include "network/Address.h"
#include "network/bundle.h"
#include "network/EndPoint.h"
#include "network/EventDispatcher.h"
#include "network/NetworkManager.h"
#include "network/PacketReceiver.h"
#include "network/error_reporter.h"

Listener::Listener(EndPoint &endpoint, Channel::ETraits traits, NetworkManager &networkMgr)
:mEndpoint(endpoint)
,mTraits(traits),
,mNetworkManager(networkMgr)
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
				WARNING_MSG(fmt::format("PacketReceiver::handleInputNotification: accept endpoint({}) {}!\n",
					 fd, kbe_strerror()));
			}

			break;
		}
		else
		{
			Channel *pChannel = Channel::ObjPool().createObject();
			bool ret = pChannel->initialize(mNetworkManager, pNewEndPoint, mTraits);
			if(!ret)
			{
				ERROR_MSG(fmt::format("Listener::handleInputNotification: initialize({}) is failed!\n",
					pChannel->c_str()));

				pChannel->destroy();
				Channel::ObjPool().reclaimObject(pChannel);
				return 0;
			}

			if(!mNetworkManager.registerChannel(pChannel))
			{
				ERROR_MSG(fmt::format("Listener::handleInputNotification: registerChannel({}) is failed!\n",
					pChannel->c_str()));

				pChannel->destroy();
				Channel::ObjPool().reclaimObject(pChannel);
			}
		}
	}

	return 0;
}
