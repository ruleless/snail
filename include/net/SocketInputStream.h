#ifndef __SOCKETINPUTSTREAM_H__
#define __SOCKETINPUTSTREAM_H__

#include "NetDef.h"
#include "Queue.h"

template<int N> class SocketInputStream : public Queue<char, N>
{
  public:
	class Listener
	{
	  public:
		Listener() {}
		virtual ~Listener() {}

		virtual void onReadFIN(int fd) = 0;
		virtual void onReadExcept(int fd) = 0;
	};
	
    SocketInputStream(int fd) : Queue<char, N>(), mSockFd(fd), mListener(NULL)
	{
	}

    virtual ~SocketInputStream() {}

	void setListener(Listener *l)
	{
		mListener = l;
	}

	void _fill()
	{
		if (N - size() <= 0)
			return;

		int n = 0;
		if (mRear >= mFront)
		{
			n = read(mSockFd, mQueue+mRear, N+1-mRear);
		}
		else
		{
			n = read(mSockFd, mQueue+mRear, mFront-mRear-1);
		}

		if (n > 0)
			mRear = (mRear+n) % (N+1);

		if (mListener)
		{
			if (0 == n)
			{
				mListener->onReadFIN(mSockFd);
			}
			else if (n < 0)
			{
				mListener->onReadExcept(mSockFd);
			}
		}
	}

	int batchRead(char *buff, int cap)
	{
		int n = min(size(), cap);
		if (n > 0)
		{
			if (mFront > mRear)
			{
				int right = min(n, N+1-mFront);
				memcpy(buff, mQueue+mFront, right);

				if (right != n)
				{
					memcpy(buff+right, mQueue, n-right);
				}
			}
			else
			{
				memcpy(buff, mQueue+mFront, n);
			}

			mFront = (mFront+n) % (N+1);
		}
		return n;
	}
  private:
	int mSockFd;
	Listener *mListener;
};

typedef SocketInputStream<BUFFSIZE> SockInputStream;

#endif
