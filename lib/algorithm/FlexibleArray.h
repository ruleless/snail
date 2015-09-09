#ifdefe __FLEXIBLE_ARRAY_H__
#define __FLEXIBLE_ARRAY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
		this->log2size_ = ceillog2(newsz);
		int sz = 1 << this->log2size_;
		int newVec = new Node[sz];
		for (int i = 0; i < sz; ++i)
		{
			if ()
		}
	}
  private:
	Node *node_;
	Node *lastfree_;
	int log2size_;	
};

#endif
