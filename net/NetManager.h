#ifndef __NETMANAGER_H__
#define __NETMANAGER_H__

#include "NetDef.h"
#include "SocketInputStream.h"
#include "SocketOutputStream.h"
#include "Accepter.h"
#include "Sender.h"
#include "Receiver.h"

#define ACCEPT_PERFRAME 100
#define EXPSOCK_SIZE 100

class NetManager : public SockInputStream::Listener, public SockOutputStream::Listener
{
  public:
	class Listener
	{
	  public:
		Listener() {}
		virtual ~Listener() {}
		
		virtual void onRecvStream(SockInputStream *inputStream) = 0;
	};
	
    NetManager();
    virtual ~NetManager();

	bool create();
	void close();

	void setListener(Listener *l)
	{
		mListener = l;
	}

	// 此方法一般放在主业务逻辑线程中调用
	void update();

	// 新连接处理
	bool _pushNewConn(const struct NewConnInfo *newConn);
	void _processNewConn();

	// 数据接收
	void _fillInputBuffer(int fd);
	void _processInputStream();

	int _lockInputBuffer(int fd);
	int _trylockInputBuffer(int fd);
	void _unlockInputBuffer(int fd);

	// 数据发送
	void _flushOutputBuffer(int fd);
	int sendData(int fd, const char *buff, int len);

	int _lockOutputBuffer(int fd);
	int _trylockOutputBuffer(int fd);
	void _unlockOutputBuffer(int fd);

	bool getFdSet(fd_set *set, int *maxFd);

	int _lockFdSet();
	void _unlockFdSet();

	// 异常连接处理
	bool _pushExpSock(int sockFd);
	void _processExpSock();

	// SockInputStream
	virtual void onReadFIN(int fd);
	virtual void onReadExcept(int fd);

	// SockOutputStream
	virtual void onWriteExcept(int fd);
  protected:
	Queue<struct NewConnInfo, ACCEPT_PERFRAME> mAcceptQueue;
	sem_t mQueFreeSize;
	sem_t mQueCurSize;

	Queue<int, EXPSOCK_SIZE> mExpSock;
	sem_t mExpSockIdle;
	sem_t mExpSockSize;
	
	int mMaxFd;
	fd_set mFdSet;
	pthread_mutex_t mFdSetMutex;

	SockInputStream* mInputBuffer[MAXFD];
	pthread_mutex_t mInputMutex[MAXFD];
	
	SockOutputStream* mOutputBuffer[MAXFD];
	pthread_mutex_t mOutputMutex[MAXFD];
	
	Accepter *mAccepter;
	Sender *mSender;
	Receiver *mReceiver;

	Listener *mListener;
};

#endif
