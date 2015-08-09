#ifndef __NETMANAGER_H__
#define __NETMANAGER_H__

#include "NetDef.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "Accepter.h"
#include "Sender.h"
#include "Receiver.h"

#define ACCEPT_PERFRAME 100

class NetManager
{
  public:
    NetManager();
    virtual ~NetManager();

	bool create();
	void close();

	// �˷���һ�������ҵ���߼��߳��е���
	void update();

	// �����Ӵ���
	bool _pushNewConn(const struct NewConnInfo *newConn);
	void _processNewConn();

	// ���ݽ���
	void _fillInputBuffer(int fd);
	void _processInputStream();

	int _lockInputBuffer(int fd);
	void _unlockInputBuffer(int fd);

	// ���ݷ���
	void _flushOutputBuffer(int fd);
	int sendData(int fd, const char *buff, int len);

	int _lockOutputBuffer(int fd);
	void _unlockOutputBuffer(int fd);

	bool getFdSet(fd_set *set, int *maxFd);
  protected:
	Queue<struct NewConnInfo, ACCEPT_PERFRAME> mAcceptQueue;
	sem_t mQueFreeSize;
	sem_t mQueCurSize;
	
	int mMaxFd;
	fd_set mFdSet;

	SockInputStream* mInputBuffer[MAXFD];
	pthread_mutex_t mInputMutex[MAXFD];
	
	SockOutputStream* mOutputBuffer[MAXFD];
	pthread_mutex_t mOutputMutex[MAXFD];

	Accepter *mAccepter;
	Sender *mSender;
	Receiver *mReceiver;
};

#endif
