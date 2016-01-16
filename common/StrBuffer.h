#ifndef __STRBUFFER_H__
#define __STRBUFFER_H__

#include "Buffer.h"

template <class Allocator>
class StrBuffer : public BasicBuffer, public Allocator
{
	typedef BasicBuffer            base;
	typedef Allocator              alct;
	typedef StrBuffer<Allocator>   ostrbuffer;
public:
	StrBuffer()
	{
		base::_data = (char*)alct::alloc(alct::init_size(), base::_cap);
	}

	StrBuffer(char* buf, size_t maxsize) : base(buf, maxsize)
	{
	}

	StrBuffer(const ostrbuffer& buf)
	{
		assign(buf);
	}

	~StrBuffer()
	{
		clear();
	}

	void clear()
	{
		if (base::data())
			alct::free(base::data());
		base::clear_ex();
	}

	const char* c_str() const
	{
		return base::_data;
	}

	ostrbuffer& operator=(const ostrbuffer& buf)
	{
		return (assign(buf));
	}

	ostrbuffer& assign(const ostrbuffer& buf)
	{
		if (!base::data())
			base::_data = (char*)alct::alloc(alct::init_size(), base::_cap);

		base::reset();
		if (buf.size() > 0)
		{
			push_back(buf.data(), buf.size());
		}
		return (*this);
	}

	ostrbuffer& push_back(const void* buf, size_t bytes)
	{
		if (buf == 0 || bytes == 0)
		{
			base::state(base::fail);
			return (*this);
		}

		if (bytes > base::right())
		{
			char* ret = (char*)alct::realloc(base::_data, base::_cur, base::_cur + bytes, base::_cap);
			if (!ret)
			{
				base::state(base::fail);
				return (*this);
			}
			base::set_data(ret);
		}

		base::copy(buf, bytes);

		return (*this);
	}

	template <class T> 
	ostrbuffer& operator<< (T value)
	{
		if (sizeof(T) > base::right())
		{
			char* ret = (char*)alct::realloc(base::_data, base::_cur, base::_cur + sizeof(T), base::_cap);
			if (!ret)
			{
				base::state(base::fail);
				return (*this);
			}
			base::set_data(ret);
		}

		if (base::current() && sizeof(T) <= right())
		{
			*(T*)base::current() = value;
			base::inc(sizeof(T));
			base::check_eof();
		}
		else
		{ 
			base::state(base::fail);
		}
		return (*this);
	}

	template <>
	ostrbuffer& operator<< (const char* value)
	{
		_trimEof();

		if (value)
			return push_back((const void*)value, strlen(value) + sizeof(char));;
		base::state(base::fail);
		return *this;
	}

	template<>
	ostrbuffer& operator<< (char* value)
	{
		return operator<< ((const char*)value);
	}

	template<> 
	ostrbuffer& operator<< (const wchar_t* value)
	{
		_trimEof();

		if (value)
			return push_back((const void*)value, (wcslen(value) + 1) * sizeof(wchar_t));
		base::state(base::fail);
		return *this;
	}

	template<> 
	ostrbuffer& operator<< (wchar_t* value)
	{
		return operator<< ((const wchar_t*)value);
	}

	template<>
	ostrbuffer& operator<< (const std::string& value)
	{
		return operator<< value.c_str();
	}

	template<>
	ostrbuffer& operator<< (std::string& value)
	{
		return operator<< value.c_str();
	}

	template<>
	ostrbuffer& operator<< (const std::wstring& value)
	{
		return operator<< value.c_str();
	}

	template<>
	ostrbuffer& operator<< (std::wstring& value)
	{
		return operator<< value.c_str();
	}

	void _trimEof()
	{
		while (base::_data && base::_cur > 0 && *(base::_data+base::_cur-1) == '\0')
		{
			base::dec(1);
		}
	}
};

typedef StrBuffer<BufferAllocatorEx<BufferAllocatorStackOrHeap<32>>>        ostrbuf32;
typedef StrBuffer<BufferAllocatorEx<BufferAllocatorStackOrHeap<64>>>        ostrbuf64;
typedef StrBuffer<BufferAllocatorEx<BufferAllocatorStackOrHeap<128>>>       ostrbuf128;
typedef StrBuffer<BufferAllocatorEx<BufferAllocatorStackOrHeap<256>>>       ostrbuf256, ostrbuf;
typedef StrBuffer<BufferAllocatorEx<BufferAllocatorStackOrHeap<512>>>       ostrbuf512;
typedef StrBuffer<BufferAllocatorEx<BufferAllocatorStackOrHeap<1024>>>      ostrbuf1024;
typedef StrBuffer<BufferAllocatorEx<BufferAllocatorStackOrHeap<2048>>>      ostrbuf2048;
typedef StrBuffer<BufferAllocatorEx<BufferAllocatorStackOrHeap<4096>>>      ostrbuf4096;
typedef StrBuffer<BufferAllocatorEx<BufferAllocatorStackOrHeap<8192>>>      ostrbuf8192;

#endif // __STRBUFFER_H__