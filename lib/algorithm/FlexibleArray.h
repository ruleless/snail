#ifdefe __FLEXIBLE_ARRAY_H__
#define __FLEXIBLE_ARRAY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/common.h"
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
	
    FlexibleArray(int log2sz = 6)
			:node_(NULL)
			,lastfree_(NULL)
			,log2size_(log2sz)
	{
		int sz = 1 << log2size_;
		this->node_ = new Node[sz];
		this->lastfree_ = &this->node_[sz-1];
		for (int i = 0; i < sz; ++i)
		{
			this->_setNil(&this->node_[i])
		}
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
		Node *n = &this->node_[key & (sz-1)]
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
		Node *n = this->getNode(key);
		if (n)
		{
			n->i_val = val;
			return &n->i_val;
		}

		int sz = 1 << this->log2size_;
		n = &this->node_[key & (sz-1)];
		if (!this->_isNil(n) && n->i_key.n != key)
		{
			Node *f = this->_getLastFree();
			if (NULL == f)
			{
				this->_resize(sz+1);
				f = this->_getLastFree();
			}
			Assert(f);

			if (n->i_key.nk.next != 0)
				f->i_key.nk.next = (n + n->i_key.nk.next) - f;
			n->i_key.nk.next = f-n;

			n = f;
		}
		n->i_val = val;
		return &n->i_val;
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

	void _resize(int newsz)
	{		
		int log2sz = ceillog2(newsz);
		Assert(log2sz > this->log2size_);		
				
		int newsz = 1 << log2sz;
		int newVec = new Node[newsz];
		int sz = 1 << this->log2size_;
		for (int i = 0; i < newsz; ++i)
		{
			if (i < sz && this->node_[i].i_key.n >= 0)
			{
				newVec[i] = this->node_[i];
			}
			else
			{
				this->_setNil(&newVec[i]);
			}
		}

		delete []this->node_;
		this->log2size_ = log2sz;
		this->node_ = newVec;
		this->lastfree_ = newVec+newsz-1;
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
