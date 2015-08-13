/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KBE_NETWORKLISTENER_RECEIVER_H
#define KBE_NETWORKLISTENER_RECEIVER_H

#include "common/common.h"
#include "common/timer.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/packet.h"
#include "network/channel.h"

namespace KBEngine { 
namespace Network
{
class EndPoint;
class Address;
class NetworkManager;
class EventDispatcher;

class ListenerReceiver : public InputNotificationHandler
{
public:
	ListenerReceiver(EndPoint & endpoint, Channel::Traits traits, NetworkManager & networkInterface);
	~ListenerReceiver();

private:
	virtual int handleInputNotification(int fd);
	EventDispatcher & dispatcher();

private:
	EndPoint & endpoint_;
	Channel::Traits traits_;
	NetworkManager & networkInterface_;
};

}
}

#ifdef _INLINE
#include "listener_receiver.inl"
#endif
#endif // KBE_NETWORKLISTENER_RECEIVER_H
