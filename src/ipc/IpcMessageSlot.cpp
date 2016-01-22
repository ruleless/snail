#include <sys/mman.h>
#include <semaphore.h>

#include "ipc/IpcMessageSlot.h"
#include "ipc/IpcMessageCommon.h"
#include "Trace.h"

IpcMessageSlot::IpcMessageSlot(const char *name, EIpcCompType type)
		:mName(name)
		,mCompType(type)
		,mCompId(getProcessPID())
		,mPendingMessage()
		,mShmFd(0)
		,mShared(NULL)
{
	memset(mHandlers, 0, sizeof(mHandlers));
}

IpcMessageSlot::~IpcMessageSlot()
{
}

bool IpcMessageSlot::open()
{
	if (IpcComp_Master == mCompType)
	{
		// 创建共享内存对象，并映射到当前进程的地址空间
		shm_unlink(mName.c_str());
		mShmFd = shm_open(mName.c_str(), O_RDWR|O_CREAT|O_EXCL, 0644);
		if (mShmFd < 0)
		{
			ErrorLn("Create share memory failed.");
			return false;
		}

		ftruncate(mShmFd, sizeof(Shared));
		mShared = (Shared *)mmap(NULL, sizeof(Shared), PROT_READ|PROT_WRITE, MAP_SHARED, mShmFd, 0);
		if (MAP_FAILED == mShared)
		{
			ErrorLn("Call mmap failed.");
			return false;
		}

		close(mShmFd);
		mShmFd = 0;

		// 初始化共享内存区中的变量
		mShared->master.comp = mCompId;
		if (!_initASlot(&mShared->master))
		{
			ErrorLn("init master slot failed.");
			return false;
		}

		for (int i = 0; i < IpcMsg_MaxSlaves; ++i)
		{
			SMessageSlot *pSlot = &mShared->slave[i];
			sem_init(&pSlot->mutex, 1, 1);
			pSlot->comp = 0;
		}
	}
	else
	{
		// 打开共享内存对象，并映射到当前进程的地址空间
		mShmFd = shm_open(mName.c_str(), O_RDWR, 0);
		if (mShmFd < 0)
		{
			ErrorLn("Open share memory failed.");
			return false;
		}

		mShared = (Shared *)mmap(NULL, sizeof(Shared), PROT_READ|PROT_WRITE, MAP_SHARED, mShmFd, 0);
		if (MAP_FAILED == mShared)
		{
			ErrorLn("Call mmap failed.");
			return false;
		}

		close(mShmFd);
		mShmFd = 0;
	}

	return true;
}

bool IpcMessageSlot::_initASlot(IpcMessageSlot::SMessageSlot *pSlot)
{
	pSlot->front = pSlot->rear = 0;
	if (sem_init(&pSlot->nEmpty, 1, IpcMsg_SlotSize) != 0)
	{
		ErrorLn("init sem:pSlot->nEmpty failed.");
		return false;
	}
	if (sem_init(&pSlot->nStored, 1, 0) != 0)
	{
		ErrorLn("init sem:pSlot->nStored failed.");
		sem_destroy(&pSlot->nEmpty);
		return false;
	}
	if (sem_init(&pSlot->nProducer, 1, 1) != 0)
	{
		ErrorLn("init sem:pSlot->nProducer failed.");
		sem_destroy(&pSlot->nEmpty);
		sem_destroy(&pSlot->nStored);
		return false;
	}

	return true;
}

void IpcMessageSlot::close()
{
	for (int i = 0; i < IpcMsg_MaxSlaves; ++i)
	{
		SMessageSlot *pSlot = &mShared->slave[i];
		sem_destroy(&pSlot->mutex);

		_uninitASlot(pSlot);
	}
	_uninitASlot(&mShared->master);
	munmap((void *)mShared, sizeof(Shared));
	mShared = NULL;
}

void IpcMessageSlot::_uninitASlot(SMessageSlot *pSlot)
{
	if (pSlot->comp != 0)
	{
		pSlot->comp = 0;
		pSlot->front = pSlot->rear = 0;
		sem_destroy(&pSlot->nEmpty);
		sem_destroy(&pSlot->nStored);
		sem_destroy(&pSlot->nProducer);
	}
}

void IpcMessageSlot::subscribe(uint8 type, IpcMessageHandler *pHandler)
{
	if (type < IpcMsg_MaxType)
	{
		mHandlers[type] = pHandler;
	}
}

void IpcMessageSlot::unsubscribe(uint8 type)
{
	if (type < IpcMsg_MaxType)
	{
		mHandlers[type] = NULL;
	}
}

void IpcMessageSlot::registerMe()
{
	if (IpcComp_Slave == mCompType)
	{
		obuf ob;
		ob<<(int)mCompId;
		postMessage(0, IPC_MSGTYPE_REG, ob.data(), ob.size());
	}
}

void IpcMessageSlot::unregisterMe()
{
	if (IpcComp_Slave == mCompType)
	{
		obuf ob;
		ob<<(int)mCompId;
		postMessage(0, IPC_MSGTYPE_UNREG, ob.data(), ob.size());
	}
}

bool IpcMessageSlot::_registerComponet(IPC_COMP_ID comp)
{
	if (IpcComp_Slave == mCompType)
	{
		ErrorLn("Can' register component on Slave.");
		return false;
	}

	for (int i = 0; i < IpcMsg_MaxSlaves; ++i)
	{
		SMessageSlot *pSlot = &mShared->slave[i];
		if (pSlot->lock())
		{
			if (0 == pSlot->comp)
			{
				pSlot->comp = comp;
				bool bOk = _initASlot(pSlot);
				pSlot->unlock();
				return bOk;
			}
			pSlot->unlock();
		}
	}

	ErrorLn("No more space to register component.");
	return false;
}

void IpcMessageSlot::_unregisterComponet(IPC_COMP_ID comp)
{
	if (IpcComp_Slave == mCompType)
	{
		ErrorLn("Can' unregister component on Slave.");
		return;
	}

	for (int i = 0; i < IpcMsg_MaxSlaves; ++i)
	{
		SMessageSlot *pSlot = &mShared->slave[i];
		if (pSlot->lock())
		{
			if (comp == pSlot->comp)
			{
				_uninitASlot(pSlot);
				pSlot->unlock();
				return;
			}
			pSlot->unlock();
		}
	}
}

void IpcMessageSlot::postMessage(IPC_COMP_ID comp, uint8 type, uint8 len, const void *buf)
{
	SIpcMessageEx msg;
	msg.comp = comp;
	msg.type = type;
	msg.len = len;
	memcpy(msg.buff, buf, len);

	mPendingMessage.push_back(msg);
}

void IpcMessageSlot::process()
{}

int IpcMessageSlot::_dispatcherMessage(SMessageSlot *slot)
{
	int count = 0;
	while (sem_trywait(&slot->nStored) == 0)
	{
		SIpcMessage *ipcMsg = &slot->msgs[slot->front];
		if (ipcMsg->type < IpcMsg_MaxType && mHandlers[ipcMsg->type])
		{
			mHandlers[ipcMsg->type]->onRecv(ipcMsg->type, ipcMsg->len, (void *)ipcMsg->buff);
		}

		slot->front = (slot->front+1)%IpcMsg_SlotSize;
		sem_post(&slot->nEmpty);
		++count;
	}
	return count;
}

void IpcMessageSlot::_processPendingMessage()
{

}

void IpcMessageSlot::onRecv(uint8 type, uint8 len, const void *buf)
{}
