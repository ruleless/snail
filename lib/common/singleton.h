#ifndef KBE_SINGLETON_H
#define KBE_SINGLETON_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "common/platform.h"
	
template <typename T> 
class Singleton
{
protected:
	static T* singleton_;

public:
	Singleton(void)
	{
		assert(!singleton_);
#if defined(_MSC_VER) && _MSC_VER < 1200	 
		int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
		singleton_ = (T*)((int)this + offset);
#else
		singleton_ = static_cast< T* >(this);
#endif
	}
	
	
	~Singleton(void){  assert(singleton_);  singleton_ = 0; }
	
	static T& getSingleton(void) { assert(singleton_);  return (*singleton_); }
	static T* getSingletonPtr(void){ return singleton_; }
};

#define KBE_SINGLETON_INIT( TYPE )							\
template <>	 TYPE * Singleton< TYPE >::singleton_ = 0;	\

#endif // KBE_SINGLETON_H
