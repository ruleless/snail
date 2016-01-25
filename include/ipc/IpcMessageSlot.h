#ifndef __IPCMESSAGESLOT_H__
#define __IPCMESSAGESLOT_H__

#include <list>
#include "IpcMessageCommon.h"

class IpcMessageSlot : public IpcMessageHandler
{
  protected:
	struct SMessageSlot
	{
		IPC_COMP_ID id;
		int front, rear;
		sem_t nEmpty, nStored, nProducer;
		SIpcMessageEx msgs[IpcMsg_SlotSize];
	};

	struct Shared
	{
		SMessageSlot master;
		SMessageSlot slave[IpcMsg_MaxSlaves];
	};

	typedef std::list<SIpcMessageEx> PendingMessage;

	std::string mName;
	EIpcCompType mCompType;
	IPC_COMP_ID mCompId;

	PendingMessage mPendingMessage;
	IpcMessageHandler* mHandlers[IpcMsg_MaxType];

	int mShmFd;
	Shared *mShared;
  public:
    IpcMessageSlot(const char *name, EIpcCompType type);
    virtual ~IpcMessageSlot();

	bool open();
	bool _initASlot(SMessageSlot *pSlot);

	void close();
	void _uninitASlot(SMessageSlot *pSlot);

	void subscribe(uint8 type, IpcMessageHandler *pHandler);
	void unsubscribe(uint8 type);

	void _registerMe();
	void _unregisterMe();

	bool _registerComponet(IPC_COMP_ID comp);
	void _unregisterComponet(IPC_COMP_ID comp);

	void postMessage(IPC_COMP_ID comp, uint8 type, uint8 len, const void *buf);

	void process();

	int _dispatchMessage(SMessageSlot *slot);
	void _processPendingMessage(bool bBlock = false);

	SMessageSlot* _findSlaveSlot(IPC_COMP_ID comp);

	virtual void onRecv(IPC_COMP_ID compId, uint8 type, uint8 len, const void *buf);
};

#endif
