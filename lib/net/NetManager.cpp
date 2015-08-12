#include "NetManager.h"

static const int s_exeTimesPerFrame = 50;  // 每帧执行的最大次数

NetManager::NetManager()
		:mMaxFd(-1)
		,mAccepter(NULL)
		,mSender(NULL)
		,mReceiver(NULL)
		,mListener(NULL)
{
	memset(mInputBuffer, 0, sizeof(mInputBuffer));
	memset(mOutputBuffer, 0, sizeof(mOutputBuffer));
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
	sem_init(&mExpSockIdle, 0, EXPSOCK_SIZE);
	sem_init(&mExpSockSize, 0, 0);

	// 互斥量初始化
	pthread_mutex_init(&mFdSetMutex, NULL);
	for (int i = 0; i < MAXFD; ++i)
	{
		pthread_mutex_init(&mInputMutex[i], NULL);
		pthread_mutex_init(&mOutputMutex[i], NULL);		
	}
	
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
	sem_destroy(&mExpSockIdle);
	sem_destroy(&mExpSockSize);

	// 互斥量销毁
	for (int i = 0; i < MAXFD; ++i)
	{
		pthread_mutex_destroy(&mInputMutex[i]);
		pthread_mutex_destroy(&mOutputMutex[i]);
	}
	pthread_mutex_destroy(&mFdSetMutex);

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
	// 断开异常连接
	_processExpSock();
	
	// 处理新连接
	_processNewConn();

	// 处理客户数据
	_processInputStream();
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
	for (int i = 0; i < s_exeTimesPerFrame; ++i)
	{
		if (sem_trywait(&mQueCurSize) < 0)
			break;
	
		NewConnInfo connInfo;
		assert(mAcceptQueue.popFront(&connInfo) && "_processNewConn pop err.");
		int fd = connInfo.fd;
		if (fd >= 0)
		{
			if (fd < MAXFD)
			{
				_lockFdSet();
				mMaxFd = max(fd, mMaxFd);
				FD_SET(fd, &mFdSet);
				_unlockFdSet();
		
				_lockInputBuffer(fd);
				SafeDelete(mInputBuffer[fd]);
				mInputBuffer[fd] = new SockInputStream(fd);
				mInputBuffer[fd]->setListener(this);
				_unlockInputBuffer(fd);

				_lockOutputBuffer(fd);
				SafeDelete(mOutputBuffer[fd]);
				mOutputBuffer[fd] = new SockOutputStream(fd);
				mOutputBuffer[fd]->setListener(this);
				_unlockOutputBuffer(fd);
			}
			else
			{
				::close(fd);
			}
		}

		sem_post(&mQueFreeSize);
	}
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
	int maxFd = -1;
	_lockFdSet();
	maxFd = mMaxFd;
	_unlockFdSet();
	
	int exeTimes = 0;
	for (int i = 0; i <= mMaxFd; ++i)
	{
		if (_trylockInputBuffer(i) < 0)
			continue;
		if (mListener && mInputBuffer[i] != NULL && !mInputBuffer[i]->empty())
		{			
			mListener->onRecvStream(mInputBuffer[i]);

			if (++exeTimes >= s_exeTimesPerFrame)
				break;
		}
		_unlockInputBuffer(i);
	}
}

int NetManager::_lockInputBuffer(int fd)
{
	return pthread_mutex_lock(&mInputMutex[fd]);
}

int NetManager::_trylockInputBuffer(int fd)
{
	return pthread_mutex_trylock(&mInputMutex[fd]);
}

void NetManager::_unlockInputBuffer(int fd)
{
	pthread_mutex_unlock(&mInputMutex[fd]);
}

void NetManager::_flushOutputBuffer(int fd)
{
	_lockOutputBuffer(fd);
	if (mOutputBuffer[fd] != NULL && !mOutputBuffer[fd]->empty())
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
	return pthread_mutex_lock(&mOutputMutex[fd]);
}

int NetManager::_trylockOutputBuffer(int fd)
{
	return pthread_mutex_trylock(&mOutputMutex[fd]);
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

int NetManager::_lockFdSet()
{
	return pthread_mutex_lock(&mFdSetMutex);
}

void NetManager::_unlockFdSet()
{
	pthread_mutex_unlock(&mFdSetMutex);
}

bool NetManager::_pushExpSock(int sockFd)
{
	if (sem_wait(&mExpSockIdle) < 0)
		return false;
	mExpSock.pushBack(sockFd);
	sem_post(&mExpSockSize);
	return true;
}

void NetManager::_processExpSock()
{
	for (int i = 0; i < s_exeTimesPerFrame; ++i)
	{
		if (sem_trywait(&mExpSockSize) < 0)
			break;

		int sockFd = -1;
		assert(mExpSock.popFront(&sockFd) && "_processExpSock Err. pop failed.");
		if (sockFd >= 0 && sockFd < MAXFD)
		{
			::close(sockFd);

			_lockInputBuffer(sockFd);
			SafeDelete(mInputBuffer[sockFd]);
			_unlockInputBuffer(sockFd);

			_lockOutputBuffer(sockFd);
			SafeDelete(mOutputBuffer[sockFd]);
			_unlockOutputBuffer(sockFd);

			_lockFdSet();
			FD_CLR(sockFd, &mFdSet);			
			if (mMaxFd == sockFd)
			{
				int fd = mMaxFd-1;
				for (; fd >= 0; --fd)
				{
					if (FD_ISSET(fd, &mFdSet))
						break;
				}
				mMaxFd = fd;
			}
			_unlockFdSet();
		}
		sem_post(&mExpSockIdle);
	}
}

void NetManager::onReadFIN(int fd)
{
	_pushExpSock(fd);
}

void NetManager::onReadExcept(int fd)
{
	_pushExpSock(fd);
}

void NetManager::onWriteExcept(int fd)
{
	_pushExpSock(fd);
}
