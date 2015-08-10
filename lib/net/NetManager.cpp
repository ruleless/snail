#include "NetManager.h"

NetManager::NetManager()
		:mMaxFd(-1)
		,mAccepter(NULL)
		,mSender(NULL)
		,mReceiver(NULL)
{
	memset(mInputBuffer, 0, sizeof(mInputBuffer));
	memset(mOutputBuffer, 0, sizeof(mOutputBuffer));

	for (int i = 0; i < MAXFD; ++i)
	{
		mInputMutex[i] = PTHREAD_MUTEX_INITIALIZER;
		mOutputMutex[i] = PTHREAD_MUTEX_INITIALIZER;
	}
}

NetManager::~NetManager()
{
	SafeDelete(mAccepter);
	SafeDelete(mSender);
	SafeDelete(mReceiver);

	for (int i = 0; i < MAXFD; ++i)
	{
		SafeDelete(mInputBuffer[i]);
		SafeDelete(mOutputBuffer[i]);
	}
}

bool NetManager::create()
{
	// 信号量初始化
	mAcceptQueue.clear();
	sem_init(&mQueFreeSize, 0, ACCEPT_PERFRAME);
	sem_init(&mQueCurSize, 0, 0);
	
	// 网络线程初始化
	mAccepter = new Accepter(this);
	mSender = new Sender(this);
	mReceiver = new Receiver(this);

	if (!mAccepter->create())
	{
		return false;
	}
	if (!mSender->create())
	{
		mAccepter->close();
		SafeDelete(mAccepter);
		return false;
	}
	if (!mReceiver->create())
	{
		mAccepter->close();
		mSender->close();
		SafeDelete(mAccepter);
		SafeDelete(mSender);
		return false;
	}	

	return true;
}

void NetManager::close()
{
	// 信号量销毁
	mAcceptQueue.clear();
	sem_destroy(&mQueFreeSize);
	sem_destroy(&mQueCurSize);

	// 网络线程销毁
	mAccepter->close();
	mSender->close();
	mReceiver->close();

	SafeDelete(mAccepter);
	SafeDelete(mSender);
	SafeDelete(mReceiver);
}

void NetManager::update()
{
}

bool NetManager::_pushNewConn(const struct NewConnInfo *newConn)
{
	if (sem_wait(&mQueFreeSize) < 0)
		return false;
	mAcceptQueue.pushBack(*newConn);
	sem_post(&mQueCurSize);
	return true;
}

void NetManager::_processNewConn()
{
	if (sem_trywait(&mQueCurSize) < 0)
		return;
	
	NewConnInfo connInfo;
	assert(mAcceptQueue.popFront(&connInfo) && "_processNewConn pop err.");
	int fd = connInfo.fd;
	if (fd >= 0 && fd < MAXFD)
	{
		mMaxFd = max(fd, mMaxFd);
		FD_SET(fd, &mFdSet);
		
		_lockInputBuffer(fd);
		SafeDelete(mInputBuffer[fd]);
		mInputBuffer[fd] = new SockInputStream(fd);
		_unlockInputBuffer(fd);

		_lockOutputBuffer(fd);
		SafeDelete(mOutputBuffer[fd]);
		mOutputBuffer[fd] = new SockOutputStream(fd);
		_unlockOutputBuffer(fd);
	}

	sem_post(&mQueFreeSize);
}

void NetManager::_fillInputBuffer(int fd)
{
	_lockInputBuffer(fd);
	if (mInputBuffer[fd] != NULL)
	{
		mInputBuffer[fd]->_fill();
	}
	_unlockInputBuffer(fd);
}

void NetManager::_processInputStream()
{	
}

int NetManager::_lockInputBuffer(int fd)
{
	pthread_mutex_lock(&mInputMutex[fd]);
}

void NetManager::_unlockInputBuffer(int fd)
{
	pthread_mutex_unlock(&mInputMutex[fd]);
}

void NetManager::_flushOutputBuffer(int fd)
{
	_lockOutputBuffer(fd);
	if (mOutputBuffer[fd] != NULL)
		mOutputBuffer[fd]->_flush();			
	_unlockOutputBuffer(fd);
}

int NetManager::sendData(int fd, const char *buff, int len)
{
	int n = 0;
	_lockOutputBuffer(fd);
	if (mOutputBuffer[fd] != NULL)
		n = mOutputBuffer[fd]->batchWrite(buff, len);
	_unlockOutputBuffer(fd);
	return n;
}

int NetManager::_lockOutputBuffer(int fd)
{
	pthread_mutex_lock(&mOutputMutex[fd]);
}

void NetManager::_unlockOutputBuffer(int fd)
{
	pthread_mutex_unlock(&mOutputMutex[fd]);
}

bool NetManager::getFdSet(fd_set *set, int *maxFd)
{
	*set = mFdSet;
	*maxFd = mMaxFd;
	return mMaxFd >= 0;
}
