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

#ifndef KBE_FIXED_NETWORK_MESSAGES_H
#define KBE_FIXED_NETWORK_MESSAGES_H

#include "common/common.h"
#include "common/Singleton.h"
#include "network/NetworkDef.h"

/*
	�������м�Э��(ǰ������֮��)����ǿ��Լ����
	û��ʹ�õ�kbe����Э���Զ��󶨻��Ƶ�ǰ�˿���ʹ�ô˴���ǿ��Լ��Э�顣
*/
class FixedMessages : public Singleton<FixedMessages>
{
public:

	// �̶���Э�����ݽṹ
	struct MSGInfo
	{
		MessageID msgid;
		std::string msgname;
		//std::wstring descr;
	};

public:
	FixedMessages();
	~FixedMessages();

	bool loadConfig(std::string fileName);

	FixedMessages::MSGInfo* isFixed(const char* msgName);
	bool isFixed(MessageID msgid);

public:
	typedef UnorderedMap<std::string, MSGInfo> MSGINFO_MAP;

private:
	MSGINFO_MAP _infomap;
	bool _loaded;
};

#endif // KBE_FIXED_NETWORK_MESSAGES_H
