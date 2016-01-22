#ifndef __IPCMESSAGESLOT_H__
#define __IPCMESSAGESLOT_H__

#include <list>
#include "IpcMessageCommon.h"

class IpcMessageSlot : public IpcMessageHandler
{
  protected:
	struct SMessageSlot
	{
		sem_t mutex;

		IPC_COMP_ID comp;
		int front, rear;
		sem_t nEmpty, nStored, nProducer;
		SIpcMessage msgs[IpcMsg_SlotSize];

		bool lock()
		{
			if (sem_wait(&mutex) == 0)
			{
				return true;
			}
			return false;
		}
		void unlock()
		{
			sem_post(&mutex);
		}
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

	void registerMe();
	void unregisterMe();

	bool _registerComponet(IPC_COMP_ID comp);
	void _unregisterComponet(IPC_COMP_ID comp);

	void postMessage(IPC_COMP_ID comp, uint8 type, uint8 len, const void *buf);

	void process();

	int _dispatcherMessage(SMessageSlot *slot);
	void _processPendingMessage();

	virtual void onRecv(uint8 type, uint8 len, const void *buf);
};

#endif
