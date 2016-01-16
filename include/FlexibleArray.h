#ifndef __FLEXIBLE_ARRAY_H__
#define __FLEXIBLE_ARRAY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "math.h"

template <typename T>
class FlexibleArray
{
  public:
	union Key
	{
		struct
		{
			int n;
			int next;
		} nk;
		int n;
	};

	struct Node
	{
		Key i_key;
		T i_val;
	};
	
    FlexibleArray(int sz = 8)
			:node_(NULL)
			,lastfree_(NULL)
			,log2size_(0)
	{
		this->_setNodeVector(sz);
	}
	
    virtual ~FlexibleArray()
	{
		delete []this->node_;
		this->node_ = NULL;
		this->lastfree_ = NULL;
		this->log2size_ = 0;
	}

	const T* get(int key)
	{
		const Node *n = this->getNode(key);
		if (n)
			return &n->i_val;
		else
			return NULL;
	}

	Node* getNode(int key)
	{
		if (key < 0)
			return NULL;

		int sz = 1 << this->log2size_;
		Node *n = &this->node_[key & (sz-1)];
		for (;;)
		{
			if (n->i_key.nk.n == key)
				return n;
			if (0 == n->i_key.nk.next)
				break;
			n += n->i_key.nk.next;
		}
		return NULL;
	}

	T* set(int key, const T & val)
	{		
		if (Node *n = this->getNode(key))
		{
			n->i_val = val;
			return &n->i_val;
		}

		int sz = 1 << this->log2size_;
		Node *mp = &this->node_[key & (sz-1)];
		if (!this->_isNil(mp) && mp->i_key.n != key)
		{
			Node *f = this->_getLastFree();
			if (NULL == f)
			{
				this->_rehash(sz+1);
				return this->set(key, val);
			}

			Node *othern = &this->node_[mp->i_key.n & (sz-1)];
			if (othern != mp)
			{
				while (othern + othern->i_key.nk.next != mp)
					othern += othern->i_key.nk.next;

				othern->i_key.nk.next = f-othern;

				*f = *mp;
				if (mp->i_key.nk.next != 0)
				{
					f->i_key.nk.next += (mp-f);
					mp->i_key.nk.next = 0;
				}
			}
			else
			{			
				if (mp->i_key.nk.next != 0)
					f->i_key.nk.next = (mp + mp->i_key.nk.next) - f;
				mp->i_key.nk.next = f-mp;
			
				mp = f;
			}
		}
		mp->i_key.nk.n = key;
		mp->i_val = val;
		return &mp->i_val;
	}

	void _setNil(Node *n)
	{
		(n->i_key).nk.n = -1;
		(n->i_key).nk.next = 0;
	}

	bool _isNil(Node *n)
	{
		return (n->i_key).nk.n < 0;
	}

	void _rehash(int sz)
	{
		Assert(math::ceillog2(sz) > this->log2size_);

		Node *oldNode = this->node_;
		int oldSz = 1 << this->log2size_;

		this->_setNodeVector(sz);
		
		for (int i = oldSz-1; i >= 0; --i)
		{
			if (!this->_isNil(&oldNode[i]))
			{
				this->set(oldNode[i].i_key.n, oldNode[i].i_val);
			}
		}
		delete []oldNode;
	}
	
	void _setNodeVector(int sz)
	{
		this->log2size_ = math::ceillog2(sz);		
		sz = 1 << this->log2size_;
		
		this->node_ = new Node[sz];
		this->lastfree_ = &this->node_[sz];
		for (int i = 0; i < sz; ++i)
		{
			this->_setNil(&this->node_[i]);
		}
	}

	Node* _getLastFree()
	{
		while (this->lastfree_ > this->node_)
		{
			--this->lastfree_;
			if (this->_isNil(this->lastfree_))
				return this->lastfree_;
		}
		return NULL;
	}
  private:
	Node *node_;
	Node *lastfree_;
	int log2size_;	
};

#endif
