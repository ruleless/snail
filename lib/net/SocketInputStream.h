#ifndef __SOCKETINPUTSTREAM_H__
#define __SOCKETINPUTSTREAM_H__

#include "NetDef.h"

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
		if (N - this->size() <= 0)
			return;

		int n = 0;
		if (this->mRear >= this->mFront)
		{
			n = read(mSockFd, this->mQueue+this->mRear, N+1-this->mRear);
		}
		else
		{
			n = read(mSockFd, this->mQueue+this->mRear, this->mFront-this->mRear-1);
		}

		if (n > 0)
			this->mRear = (this->mRear+n) % (N+1);

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
		int n = min(this->size(), cap);
		if (n > 0)
		{
			if (this->mFront > this->mRear)
			{
				int right = min(n, N+1-this->mFront);
				memcpy(buff, this->mQueue+this->mFront, right);

				if (right != n)
				{
					memcpy(buff+right, this->mQueue, n-right);
				}
			}
			else
			{
				memcpy(buff, this->mQueue+this->mFront, n);
			}

			this->mFront = (this->mFront+n) % (N+1);
		}
		return n;
	}
  protected:
	int mSockFd;
	Listener *mListener;
};

typedef SocketInputStream<BUFFSIZE> SockInputStream;

#endif
