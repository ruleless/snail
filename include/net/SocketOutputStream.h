#ifndef __SOCKETOUTPUTSTREAM_H__
#define __SOCKETOUTPUTSTREAM_H__

#include "NetDef.h"

template<int N> class SocketOutputStream : public Queue<char, N>
{
  public:
	class Listener
	{
	  public:
		Listener() {}
		virtual ~Listener() {}

		virtual void onWriteExcept(int fd);
	};
	
    SocketOutputStream(int fd) : Queue<char, N>(), mSockFd(fd), mListener(NULL)
	{
	}
	
	virtual ~SocketOutputStream() {}

	void setListener(Listener *l)
	{
		mListener = l;
	}

	void _flush()
	{
		if (size() == 0)
			return;

		int writeLen = 0;
		if (mFront > mRear)
		{
			int s = size();
			int right = min(s, N+1-mFront);
			int n = write(mSockFd, mQueue+mFront, right);

			if (n > 0)
			{
				writeLen += n;

				if (s != right)
				{
					n = write(mSockFd, mQueue, s-right);
					if (n > 0)
					{
						writeLen += n;
					}
				}
			}
		}
		else
		{
			n = write(mSockFd, mQueue+mFront, size());
		}

		if (n != size() && mListener)
		{
			mListener->onWriteExcept(mSockFd);
		}

		if (writeLen > 0)
			mFront = (mFront+writeLen) % (N+1);
	}

	int batchWrite(const char *buff, int len)
	{
		if (full())
			return 0;

		int writeLen = 0;
		if (mRear >= mFront)
		{
			int n = min(len, N+1-mRear);
			memcpy(mQueue+mRear, buff, n);
			writeLen += n;

			if (n != len)
			{
				n = min(len-n, mFront-1);
				if (n > 0)
				{
					memcpy(mQueue, buff+writeLen, n);
					writeLen += n;
				}
			}
		}
		else
		{
			int n = min(len, mFront-mRear-1);
			memcpy(mQueue+mRear, buff, n);
		}
		
		mRear = (mRear+writeLen) % (N+1);
		return writeLen;
	}
  private:
	int mSockFd;
	Listener *mListener;
};

typedef SocketOutputStream<BUFFSIZE> SockOutputStream;

#endif
