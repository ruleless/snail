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
		,mbOpen(false)
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
	if (mbOpen)
	{
		ErrorLn("IpcMessageSlot already opened.");
		return true;
	}

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

		// 初始化共享内存区中的变量
		if (!_initASlot(&mShared->master))
		{
			ErrorLn("init master slot failed.");
			return false;
		}
		mShared->master.id = mCompId;

		for (int i = 0; i < IpcMsg_MaxSlaves; ++i)
		{
			SMessageSlot *pSlot = &mShared->slave[i];
			pSlot->id = 0;
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

		// 向主进程注册
		_registerMe();
	}

	mbOpen = true;
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
	if (!mbOpen)
	{
		ErrorLn("IpcMessageSlot already closed.");
		return;
	}

	if (IpcComp_Master == mCompType)
	{
		for (int i = 0; i < IpcMsg_MaxSlaves; ++i)
		{
			SMessageSlot *pSlot = &mShared->slave[i];
			if (pSlot->id != 0)
			{
				ErrorLn("IpcMessageSlot::close()  there is a slave unreged! slave="<<pSlot->id);
			}

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

	::close(mShmFd);
	mShmFd = 0;
	if (IpcComp_Master == mCompType)
	{
		shm_unlink(mName.c_str());
	}

	mbOpen = false;
}

void IpcMessageSlot::_uninitASlot(SMessageSlot *pSlot)
{
	if (pSlot->id != 0)
	{
		pSlot->id = 0;
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

void IpcMessageSlot::_registerMe()
{
	if (IpcComp_Slave == mCompType)
	{
		postMessage(0, IPC_MSGTYPE_REG, 0, NULL);
	}
}

void IpcMessageSlot::_unregisterMe()
{
	if (IpcComp_Slave == mCompType)
	{
		postMessage(0, IPC_MSGTYPE_UNREG, 0, NULL);
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
		if (0 == pSlot->id)
		{
			if (_initASlot(pSlot))
			{
				pSlot->id = comp;
				return true;
			}
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
		if (comp == pSlot->id)
		{
			return _uninitASlot(pSlot);
		}
	}
}

void IpcMessageSlot::postMessage(IPC_COMP_ID comp, uint8 type, uint8 len, const void *buf)
{
	SIpcMessageEx msg;
	msg.source = mCompId;
	msg.dest = comp;
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
		SMessageSlot *pSlot = _findSlaveSlot(mCompId);
		if (pSlot)
		{
			_dispatchMessage(pSlot);
		}
	}
}

int IpcMessageSlot::_dispatchMessage(SMessageSlot *slot)
{
	int count = 0;
	while (sem_trywait(&slot->nStored) == 0)
	{
		SIpcMessageEx *ipcMsg = &slot->msgs[slot->front];
		if (ipcMsg->type < IpcMsg_MaxType && mHandlers[ipcMsg->type])
		{
			mHandlers[ipcMsg->type]->onRecv(ipcMsg->source, ipcMsg->type, ipcMsg->len, (void *)ipcMsg->buff);
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
			pSlot = _findSlaveSlot(msg.dest);
			if (NULL == pSlot)
			{
				ErrorLn("Can' find dest component! dest="<<msg.dest);
				continue;
			}
		}
		else // 从进程只能向主进程发送数据
		{
			if (mShared->master.id != 0)
			{
				pSlot = &mShared->master;
			}
			if (NULL == pSlot)
			{
				ErrorLn("Master Component is not initilised.");
				continue;
			}
		}

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

			sem_post(&pSlot->nStored);

			it = mPendingMessage.erase(it);
		}
		else
		{
			++it;
		}
	}
}

IpcMessageSlot::SMessageSlot* IpcMessageSlot::_findSlaveSlot(IPC_COMP_ID comp) const
{
	for (int i = 0; i < IpcMsg_MaxSlaves; ++i)
	{
		if (mShared->slave[i].id == comp)
		{
			return &mShared->slave[i];
		}
	}
	return NULL;
}

void IpcMessageSlot::onRecv(IPC_COMP_ID source, uint8 type, uint8 len, const void *buf)
{
	assert(source != 0 && "source != 0");

	switch (type)
	{
	case IPC_MSGTYPE_REG:
		{
			_registerComponet(source);
		}
		break;
	case IPC_MSGTYPE_UNREG:
		{
			_unregisterComponet(source);
		}
		break;
	default:
		break;
	}
}
