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

		virtual void onWriteExcept(int fd) = 0;
	};
	
    SocketOutputStream(int fd) : Queue<char, N>(), mSockFd(fd), mListener(NULL)
	{
	}
	
	virtual ~SocketOutputStream() {}

	void setListener(Listener *l)
	{
		mListener = l;
	}

	int getSockFd() const
	{
		return mSockFd;
	}

	void _flush()
	{
		if (this->size() == 0)
			return;

		int writeLen = 0;
		int n = 0;
		if (this->mFront > this->mRear)
		{
			int s = this->size();
			int right = min(s, N+1-this->mFront);
			n = write(mSockFd, this->mQueue+this->mFront, right);

			if (n > 0)
			{
				writeLen += n;

				if (s != right)
				{
					n = write(mSockFd, this->mQueue, s-right);
					if (n > 0)
					{
						writeLen += n;
					}
				}
			}
		}
		else
		{
			n = write(mSockFd, this->mQueue+this->mFront, this->size());
			if (n > 0)
			{
				writeLen += n;
			}
		}

		if (n != this->size() && mListener)
		{
			mListener->onWriteExcept(mSockFd);
		}

		if (writeLen > 0)
			this->mFront = (this->mFront+writeLen) % (N+1);
	}

	int batchWrite(const char *buff, int len)
	{
		if (this->full())
			return 0;

		int writeLen = 0;
		if (this->mRear >= this->mFront)
		{
			int n = min(len, N+1-this->mRear);
			memcpy(this->mQueue+this->mRear, buff, n);
			writeLen += n;

			if (n != len)
			{
				n = min(len-n, this->mFront-1);
				if (n > 0)
				{
					memcpy(this->mQueue, buff+writeLen, n);
					writeLen += n;
				}
			}
		}
		else
		{
			int n = min(len, this->mFront-this->mRear-1);
			memcpy(this->mQueue+this->mRear, buff, n);
		}
		
		this->mRear = (this->mRear+writeLen) % (N+1);
		return writeLen;
	}
  private:
	int mSockFd;
	Listener *mListener;
};

typedef SocketOutputStream<BUFFSIZE> SockOutputStream;

#endif
