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

	ostrbuffer& operator<< (int64 n)
	{
		char num[32] = {0};
		__snprintf(num, sizeof(num), "%"PRI64, n);
		return this->operator<<((const char *)num);
	}
	ostrbuffer& operator<< (uint64 n)
	{
		char num[32] = {0};
		__snprintf(num, sizeof(num), "%"PRIu64, n);
		return this->operator<<((const char *)num);
	}
	ostrbuffer& operator<< (int n)
	{
		char num[32] = {0};
		__snprintf(num, sizeof(num), "%d", n);
		return this->operator<<((const char *)num);
	}
	ostrbuffer& operator<< (uint n)
	{
		char num[32] = {0};
		__snprintf(num, sizeof(num), "%u", n);
		return this->operator<<((const char *)num);
	}
	ostrbuffer& operator<< (short n)
	{
		char num[32] = {0};
		__snprintf(num, sizeof(num), "%d", n);
		return this->operator<<((const char *)num);
	}
	ostrbuffer& operator<< (ushort n)
	{
		char num[32] = {0};
		__snprintf(num, sizeof(num), "%u", n);
		return this->operator<<((const char *)num);
	}

	ostrbuffer& operator<< (const char* value)
	{
		this->_trimEof();

		if (value)
			return this->push_back((const void*)value, strlen(value) + sizeof(char));;
		base::state(base::fail);
		return *this;
	}

	ostrbuffer& operator<< (char* value)
	{
		return this->operator<<((const char*)value);
	}

	ostrbuffer& operator<< (const wchar_t* value)
	{
		_trimEof();

		if (value)
			return this->push_back((const void*)value, (wcslen(value) + 1) * sizeof(wchar_t));
		base::state(base::fail);
		return *this;
	}

	ostrbuffer& operator<< (wchar_t* value)
	{
		return this->operator<<((const wchar_t*)value);
	}

	ostrbuffer& operator<< (const std::string& value)
	{
		return this->operator<<(value.c_str());
	}

	ostrbuffer& operator<< (std::string& value)
	{
		return this->operator<<(value.c_str());
	}

	ostrbuffer& operator<< (const std::wstring& value)
	{
		return this->operator<<(value.c_str());
	}

	ostrbuffer& operator<< (std::wstring& value)
	{
		return this->operator<<(value.c_str());
	}

	void _trimEof()
	{
		while (base::_data && base::_cur > 0 && *(base::_data+base::_cur-1) == '\0')
		{
			base::dec(1);
		}
	}
};

typedef StrBuffer<BufferAlctEx32>        ostrbuf32;
typedef StrBuffer<BufferAlctEx64>        ostrbuf64;
typedef StrBuffer<BufferAlctEx128>       ostrbuf128;
typedef StrBuffer<BufferAlctEx256>       ostrbuf256, ostrbuf;
typedef StrBuffer<BufferAlctEx512>       ostrbuf512;
typedef StrBuffer<BufferAlctEx1024>      ostrbuf1024;
typedef StrBuffer<BufferAlctEx2048>      ostrbuf2048;
typedef StrBuffer<BufferAlctEx4096>      ostrbuf4096;
typedef StrBuffer<BufferAlctEx8192>      ostrbuf8192;

#endif // __STRBUFFER_H__
