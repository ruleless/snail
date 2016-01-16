#ifndef __OBJECTPOOL_H__
#define __OBJECTPOOL_H__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <map>	
#include <list>	
#include <vector>
#include <queue> 

#include "ThreadMutex.h"

#define OBJECT_POOL_INIT_SIZE	16
#define OBJECT_POOL_INIT_MAX_SIZE	OBJECT_POOL_INIT_SIZE*16

// 对象池管理容器
template<typename T> class ObjectPool
{
  public:
	typedef std::list<T *> OBJECTS;

	ObjectPool(std::string name)
			:mName(name)
			,mMutex()
			,mIsDestroyed(false)
			,mObjects()
			,mTotalAllocs(0)
			,mObjCount(0)
			,mMaxSize(OBJECT_POOL_INIT_MAX_SIZE)
	{
	}

	ObjectPool(std::string name, unsigned int preAssignVal, size_t max)
			:mObjects()
			,mMaxSize((max == 0 ? 1 : max))
			,mIsDestroyed(false)
			,mMutex()
			,mName(name)
			,mTotalAllocs(0)
			,mObjCount(0)
	{
	}

	~ObjectPool()
	{
		destroy();
	}	
	
	void destroy()
	{
		mMutex.lockMutex();

		mIsDestroyed = true;

		typename OBJECTS::iterator iter = mObjects.begin();
		for(; iter != mObjects.end(); ++iter)
		{
			if(!(*iter)->destructorPoolObject())
			{
				delete (*iter);
			}
		}
				
		mObjects.clear();
		mObjCount = 0;

		mMutex.unlockMutex();
	}

	const OBJECTS& objects() const
	{
		return mObjects; 
	}

	void assignObjs(int preAssignVal = OBJECT_POOL_INIT_SIZE)
	{
		for(int i = 0; i < preAssignVal; ++i)
		{
			mObjects.push_back(new T);
			++mTotalAllocs;
			++mObjCount;
		}
	}

	template<typename T1> T* createObject()
	{
		mMutex.lockMutex();

		while(true)
		{
			if(mObjCount > 0)
			{
				T* t = static_cast<T1 *>(*mObjects.begin());
				mObjects.pop_front();

				--mObjCount;
				t->onEabledPoolObject();

				mMutex.unlockMutex();
				return t;
			}

			assignObjs();
		}

		mMutex.unlockMutex();

		return NULL;
	}

	T* createObject()
	{
		mMutex.lockMutex();

		while(true)
		{
			if(mObjCount > 0)
			{
				T* t = static_cast<T *>(*mObjects.begin());
				mObjects.pop_front();

				--mObjCount;
				t->onEabledPoolObject();

				mMutex.unlockMutex();
				return t;
			}

			assignObjs();
		}

		mMutex.unlockMutex();

		return NULL;
	}

	// 回收一个对象
	void reclaimObject(T *obj)
	{
		mMutex.lockMutex();
		_reclaimObject(obj);
		mMutex.unlockMutex();
	}

	// 回收一个对象容器
	void reclaimObject(std::list<T *> &objs)
	{
		mMutex.lockMutex();

		typename std::list<T *>::iterator iter = objs.begin();
		for(; iter != objs.end(); ++iter)
		{
			_reclaimObject((*iter));
		}
		
		objs.clear();

		mMutex.unlockMutex();
	}

	void reclaimObject(std::vector<T *> &objs)
	{
		mMutex.lockMutex();

		typename std::vector<T *>::iterator iter = objs.begin();
		for(; iter != objs.end(); ++iter)
		{
			_reclaimObject((*iter));
		}
		
		objs.clear();

		mMutex.unlockMutex();
	}

	void reclaimObject(std::queue<T *> &objs)
	{
		mMutex.lockMutex();

		while(!objs.empty())
		{
			T* t = objs.front();
			objs.pop();
			_reclaimObject(t);
		}

		mMutex.unlockMutex();
	}

	size_t size(void) const
	{
		return mObjCount; 
	}
	
	std::string c_str()
	{
		char buf[1024];

		mMutex.lockMutex();

		sprintf(buf, "ObjectPool::c_str(): name=%s, objs=%d/%d, isDestroyed=%s.\n", 
				mName.c_str(), (int)mObjCount, (int)mMaxSize, (isDestroyed ? "true" : "false"));

		mMutex.unlockMutex();

		return buf;
	}

	size_t maxSize() const
	{
		return mMaxSize; 
	}
	size_t totalAllocs() const
	{
		return mTotalAllocs;
	}
	bool isDestroyed() const
	{
		return mIsDestroyed; 
	}
  protected:
	void _reclaimObject(T* obj)
	{
		if(obj != NULL)
		{
			// 先重置状态
			obj->onReclaimObject();

			if(size() >= mMaxSize || mIsDestroyed)
			{
				delete obj;
				--mTotalAllocs;
			}
			else
			{
				mObjects.push_back(obj);
				++mObjCount;
			}
		}
	}
  protected:
	std::string mName;
	ThreadMutex mMutex;
	bool mIsDestroyed;

	OBJECTS mObjects;
	size_t mTotalAllocs;
	size_t mObjCount;

	size_t mMaxSize;
};

// 池对象
class PoolObject
{
  public:
	virtual ~PoolObject() {}
	virtual void onReclaimObject() = 0;
	virtual void onEabledPoolObject() {}

	virtual size_t getPoolObjectBytes()
	{
		return 0;
	}

	virtual bool destructorPoolObject()
	{
		return false;
	}
};

// 智能池对象
template<typename T> class SmartPoolObject
{
  public:
	SmartPoolObject(T *pPoolObject, ObjectPool<T> &objectPool)
			:mPoolObject(pPoolObject)
			,mObjectPool(objectPool)
	{
	}

	~SmartPoolObject()
	{
		onReclaimObject();
	}

	void onReclaimObject()
	{
		if(mPoolObject != NULL)
		{
			mObjectPool.reclaimObject(mPoolObject);
			mPoolObject = NULL;
		}
	}

	T* get()
	{
		return mPoolObject;
	}

	T* operator->()
	{
		return mPoolObject;
	}

	T& operator*()
	{
		return *mPoolObject;
	}
  private:
	T* mPoolObject;
	ObjectPool<T> &mObjectPool;
};

#define NEW_POOL_OBJECT(TYPE) TYPE::ObjPool().createObject();

#endif
