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
		if (!_initASlot(&mShared->master))
		{
			ErrorLn("init master slot failed.");
			return false;
		}
		mShared->master.comp = mCompId;

		for (int i = 0; i < IpcMsg_MaxSlaves; ++i)
		{
			SMessageSlot *pSlot = &mShared->slave[i];
			pSlot->comp = 0;
		}

		// 注册回调
		subscribe(IPC_MSGTYPE_REG, this);
		subscribe(IPC_MSGTYPE_UNREG, this);
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

	if (sem_init(&pSlot->bReadyToDispatch, 1, 1) != 0)
	{
		ErrorLn("init sem:pSlot->bReadyToDispatch failed.");
		sem_destroy(&pSlot->nEmpty);
		sem_destroy(&pSlot->nStored);
		sem_destroy(&pSlot->nProducer);
		return false;
	}

	return true;
}

void IpcMessageSlot::close()
{
	if (IpcComp_Master == mCompType)
	{
		for (int i = 0; i < IpcMsg_MaxSlaves; ++i)
		{
			SMessageSlot *pSlot = &mShared->slave[i];

			_uninitASlot(pSlot);
		}
		_uninitASlot(&mShared->master);

		// 反注册回调
		unsubscribe(IPC_MSGTYPE_REG);
		unsubscribe(IPC_MSGTYPE_UNREG);
	}
	else
	{
		_unregisterMe();
		_processPendingMessage(true);
	}

	munmap((void *)mShared, sizeof(Shared));
	mShared = NULL;
}

void IpcMessageSlot::_uninitASlot(SMessageSlot *pSlot)
{
	if (pSlot->comp != 0)
	{
		pSlot->comp = 0;
		pSlot->front = pSlot->rear = 0;
		sem_destroy(&pSlot->bReadyToDispatch);
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

void IpcMessageSlot::_registerMe()
{
	if (IpcComp_Slave == mCompType)
	{
		obuf ob;
		ob<<(int)mCompId;
		postMessage(0, IPC_MSGTYPE_REG, ob.data(), ob.size());
	}
}

void IpcMessageSlot::_unregisterMe()
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
		if (0 == pSlot->comp)
		{
			pSlot->comp = comp;
			return _initASlot(pSlot);
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
		if (comp == pSlot->comp)
		{
			return _uninitASlot(pSlot);
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
{
	// 消息入队
	_processPendingMessage(false);

	// 消息分发
	if (IpcComp_Master == mCompType)
	{
		_dispatchMessage(&mShared->master);
	}
	else
	{
	}
}

int IpcMessageSlot::_dispatchMessage(SMessageSlot *slot)
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

void IpcMessageSlot::_processPendingMessage(bool bBlock)
{
	PendingMessage::iterator it = mPendingMessage.begin();
	while (it != mPendingMessage.end())
	{
		const SIpcMessageEx &msg = *it;
		SMessageSlot *pSlot = NULL;
		if (IpcComp_Master == mCompType) // 主进程可向所有从进程发送数据
		{
			pSlot = _findSlaveSlot(msg.comp);
		}
		else // 从进程只能向主进程发送数据
		{
			pSlot = &mShared->master;
		}

		if (NULL == pSlot)
		{
			ostrbuf strLog;
			strLog<<"Can' find component! compid="<<msg.comp;
			ErrorLn(strLog.c_str());

			it = mPendingMessage.erase(it);
		}
		else
		{
			bool bGetLock = false;
			if (bBlock)
			{
				bGetLock = sem_wait(&pSlot->nEmpty) == 0;
			}
			else
			{
				bGetLock = sem_trywait(&pSlot->nEmpty) == 0;
			}

			if (bGetLock)
			{
				sem_wait(&pSlot->nProducer);
				pSlot->msgs[pSlot->rear] = msg;
				pSlot->rear = (pSlot->rear+1)%IpcMsg_SlotSize;
				sem_post(&pSlot->nProducer);

				sem_post(pSlot->nStored);

				it = mPendingMessage(it);
			}
			else
			{
				++it;
			}
		}
	}
}

SMessageSlot* IpcMessageSlot::_findSlaveSlot(IPC_COMP_ID comp)
{
	for (int i = 0; i < IpcMsg_MaxSlaves; ++i)
	{
		if (mShared->slave[i].comp == comp)
		{
			return &mShared->slave[i];
		}
	}
	return NULL;
}

void IpcMessageSlot::onRecv(uint8 type, uint8 len, const void *buf)
{}
