#ifndef __REFCOUNTABLE_H__
#define __REFCOUNTABLE_H__
	
#include "common.h"

class RefCountable 
{
public:
	void incRef(void) const
	{
		++refCount_;
	}

	void decRef(void) const
	{
		
		int currRef = --refCount_;
		assert(currRef >= 0 && "RefCountable:currRef maybe a error!");
		if (0 >= currRef)
			onRefOver();
	}

	virtual void onRefOver(void) const
	{
		delete const_cast<RefCountable *>(this);
	}

	void setRefCount(int n)
	{
		refCount_ = n;
	}

	int getRefCount(void) const 
	{ 
		return refCount_; 
	}
protected:
	RefCountable(void) : refCount_(0) 
	{
	}

	virtual ~RefCountable(void) 
	{ 
		assert(0 == refCount_ && "RefCountable:currRef maybe a error!"); 
	}
protected:
	volatile mutable long refCount_;
};

#if PLATFORM == PLATFORM_WIN32
class SafeRefCountable 
{
public:
	void incRef(void) const
	{
		::InterlockedIncrement(&refCount_);
	}

	void decRef(void) const
	{
		
		long currRef =::InterlockedDecrement(&refCount_);
		assert(currRef >= 0 && "RefCountable:currRef maybe a error!");
		if (0 >= currRef)
			onRefOver();											// 引用结束了
	}

	virtual void onRefOver(void) const
	{
		delete const_cast<SafeRefCountable*>(this);
	}

	void setRefCount(long n)
	{
		InterlockedExchange((long *)&refCount_, n);
	}

	int getRefCount(void) const 
	{ 
		return InterlockedExchange((long *)&refCount_, refCount_);
	}
protected:
	SafeRefCountable(void) : refCount_(0) 
	{
	}

	virtual ~SafeRefCountable(void) 
	{ 
		assert(0 == refCount_ && "SafeRefCountable:currRef maybe a error!"); 
	}

protected:
	volatile mutable long refCount_;
};
#else
class SafeRefCountable 
{
public:
	void incRef(void) const
	{
		__asm__ volatile (
			"lock addl $1, %0"
			:						// no output
			: "m"	(this->refCount_) 	// input: this->count_
			: "memory" 				// clobbers memory
		);
	}

	void decRef(void) const
	{
		
		long currRef = intDecRef();
		assert(currRef >= 0 && "RefCountable:currRef maybe a error!");
		if (0 >= currRef)
			onRefOver();
	}

	virtual void onRefOver(void) const
	{
		delete const_cast<SafeRefCountable*>(this);
	}

	void setRefCount(long n)
	{
		//InterlockedExchange((long *)&refCount_, n);
	}

	int getRefCount(void) const 
	{ 
		//return InterlockedExchange((long *)&refCount_, refCount_);
		return refCount_;
	}

protected:
	SafeRefCountable(void) : refCount_(0) 
	{
	}

	virtual ~SafeRefCountable(void) 
	{ 
		assert(0 == refCount_ && "SafeRefCountable:currRef maybe a error!"); 
	}
protected:
	volatile mutable long refCount_;
private:
	/**
	 *	This private method decreases the reference count by 1.
	 */
	int intDecRef() const
	{
		int ret;
		__asm__ volatile (
			"mov $-1, %0  \n\t"
			"lock xadd %0, %1"
			: "=&a"	(ret)				// output only and early clobber
			: "m"	(this->refCount_)		// input (memory)
			: "memory"
		);
		return ret;
	}
};
#endif

template<class T>
class RefCountedPtr 
{
public:
	RefCountedPtr(T* ptr) : ptr_(ptr) 
	{
		if (ptr_)
			ptr_->addRef();
	}

	RefCountedPtr(RefCountedPtr<T>* refptr) : ptr_(refptr->getObject()) 
	{
		if (ptr_)
			ptr_->addRef();
	}
	
	~RefCountedPtr(void) 
	{
		if (0 != ptr_)
			ptr_->decRef();
	}

	T& operator*() const 
	{ 
		return *ptr_; 
	}

	T* operator->() const 
	{ 
		return (&**this); 
	}

	T* getObject(void) const 
	{ 
		return ptr_; 
	}

private:
	T* ptr_;
};

#endif // __REFCOUNTABLE_H__
