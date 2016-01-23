#ifndef __IPCMESSAGECOMMON_H__
#define __IPCMESSAGECOMMON_H__

#include "common.h"

enum EIpcMessage
{
	IpcMsg_MaxType = 255,
	IpcMsg_MessageSize = 255,

	IpcMsg_MaxSlaves = 16,

	IpcMsg_SlotSize = 16,
};

enum EIpcCompType
{
	IpcComp_Master,
	IpcComp_Slave,
};

struct SIpcMessage
{
    uint8 type;
	uint8 len;
	char buff[IpcMsg_MessageSize];

	SIpcMessage()
	{
		type = len = 0;
	}

	SIpcMessage& operator=(const SIpcMessage &msg)
	{
		this->type = msg.type;
		this->len = min(msg.len, IpcMsg_MessageSize);
		memcpy(this->buff, msg.buff, this->len);
	}
};

#define IPC_COMP_ID pid_t
struct SIpcMessageEx : public SIpcMessage
{
	IPC_COMP_ID comp;

	SIpcMessageEx() : SIpcMessage()
	{
		comp = 0;
	}
};

#define IPC_MSGTYPE_REG      224
#define IPC_MSGTYPE_UNREG    225

class IpcMessageHandler
{
  public:
	virtual void onRecv(uint8 type, uint8 len, const void *buf) = 0;
};

#endif
