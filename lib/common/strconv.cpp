#include "strconv.h"

#if PLATFORM == PLATFORM_WIN32
#	include <windows.h>
#endif


template <class _CharType>
inline void strconv_allocMemory(_CharType** ppBuff, int length, _CharType* pszFixedBuffer, int fixedBufferLength)
{
	assert(ppBuff != NULL);
	assert(length >= 0);
	if (pszFixedBuffer == NULL)
	{
		_CharType* pReallocBuf = static_cast< _CharType*>(_recalloc(*ppBuff, length, sizeof(_CharType)));
		*ppBuff = pReallocBuf;
		return;
	}

	if (*ppBuff != pszFixedBuffer)
	{
		if( length > fixedBufferLength )
		{
			_CharType* pReallocBuf = static_cast< _CharType*>(_recalloc(*ppBuff, length, sizeof(_CharType)));
			*ppBuff = pReallocBuf;
		} 
		else
		{
			free(*ppBuff);
			*ppBuff = pszFixedBuffer;
		}
	}
	else
	{
		if( length > fixedBufferLength )
		{
			*ppBuff = static_cast< _CharType* >(calloc(length, sizeof(_CharType)));
		}
		else
		{			
			*ppBuff=pszFixedBuffer;
		}
	}
}

template <class _CharType>
inline void strconv_freeMemory(_CharType* pBuff,_CharType* pszFixedBuffer,int /*fixedBufferLength*/)
{
	if (pBuff != pszFixedBuffer)
	{
		free( pBuff );
	} 	
}


strconv_a2w::strconv_a2w(const char* sz, unsigned int codePage)
:m_psz(m_szBuffer)
{
	conv(sz, codePage);
}

strconv_a2w::~strconv_a2w()
{
	strconv_freeMemory(m_psz, m_szBuffer, FIXED_CONV_BUF_LEN);
}


void strconv_a2w::conv(const char* sz, unsigned int codePage)
{
	if (!sz)
	{
		m_psz = 0;
		return;
	}

#if PLATFORM == PLATFORM_WIN32
	if (codePage == (unsigned int)-1)
		codePage = ::GetACP();
#endif

	int lengthA = strlen(sz) + 1;
	int lengthW = lengthA;

	strconv_allocMemory(&m_psz, lengthW, m_szBuffer, FIXED_CONV_BUF_LEN);

#if PLATFORM == PLATFORM_WIN32
	if (0 == ::MultiByteToWideChar(codePage, 0, sz, lengthA, m_psz, lengthW))
	{
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) // 缓冲区不足
		{
			lengthW = ::MultiByteToWideChar( codePage, 0, sz, lengthA, NULL, 0);
			strconv_allocMemory(&m_psz,lengthW,m_szBuffer,FIXED_CONV_BUF_LEN);
			::MultiByteToWideChar(codePage, 0, sz, lengthA, m_psz, lengthW);
		}			
	}
#elif PLATFORM == PLATFORM_UNIX
	mbstowcs(m_psz, sz, lengthA);
#endif
}

wchar_t* conv_a2w(const char* sz, unsigned int codePage)
{
	wchar_t* wstr = 0;
	if (!sz)
	{
		return wstr;
	}

#if PLATFORM == PLATFORM_WIN32
	if (codePage == (unsigned int)-1)
		codePage = ::GetACP();
#endif

	int lengthA = strlen(sz) + 1;
	int lengthW = lengthA;

	strconv_allocMemory(&wstr, lengthW, (wchar_t*)0, 0);

#if PLATFORM == PLATFORM_WIN32
	if (0 == ::MultiByteToWideChar(codePage, 0, sz, lengthA, wstr, lengthW))
	{
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) // 缓冲区不足
		{
			lengthW = ::MultiByteToWideChar( codePage, 0, sz, lengthA, NULL, 0);
			strconv_allocMemory(&wstr, lengthW, (wchar_t*)0, 0);
			::MultiByteToWideChar(codePage, 0, sz, lengthA, wstr, lengthW);
		}			
	}
#elif PLATFORM == PLATFORM_UNIX
	mbstowcs(wstr, sz, lengthA);
#endif
	return wstr;
}


strconv_w2a::strconv_w2a(const wchar_t* sz, unsigned int codePage)
:m_psz(m_szBuffer)
{
	conv(sz, codePage);
}

strconv_w2a::~strconv_w2a()
{
	strconv_freeMemory(m_psz, m_szBuffer, FIXED_CONV_BUF_LEN);
}

void strconv_w2a::conv(const wchar_t* sz, unsigned int codePage)
{
	if (!sz)
	{
		m_psz = 0;
		return;
	}

#if PLATFORM == PLATFORM_WIN32
	if (codePage == (unsigned int)-1)
		codePage = ::GetACP();
#endif

	int lengthW = wcslen(sz) + 1;
	int lengthA = lengthW * 4;

	strconv_allocMemory(&m_psz, lengthA, m_szBuffer, FIXED_CONV_BUF_LEN);

#if PLATFORM == PLATFORM_WIN32
	if (0 == ::WideCharToMultiByte(codePage, 0, sz, lengthW, m_psz, lengthA, NULL, NULL))
	{
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) // 缓冲区不足
		{
			lengthW = ::WideCharToMultiByte( codePage, 0, sz, lengthW, NULL, 0, NULL, NULL);
			strconv_allocMemory(&m_psz,lengthW,m_szBuffer,FIXED_CONV_BUF_LEN);
			::WideCharToMultiByte(codePage, 0, sz, lengthW, m_psz, lengthA, NULL, NULL);
		}			
	}
#elif PLATFORM == PLATFORM_UNIX
	wcstombs(m_psz, sz, lengthA);
#endif
}

char* conv_w2a(const wchar_t* sz, unsigned int codePage)
{
	char* str = 0;
	if (!sz)
	{
		return str;
	}

#if PLATFORM == PLATFORM_WIN32
	if (codePage == (unsigned int)-1)
		codePage = ::GetACP();
#endif

	int lengthW = wcslen(sz) + 1;
	int lengthA = lengthW * 4;

	strconv_allocMemory(&str, lengthA, (char*)0, 0);

#if PLATFORM == PLATFORM_WIN32
	if (0 == ::WideCharToMultiByte(codePage, 0, sz, lengthW, str, lengthA, NULL, NULL))
	{
		if (::GetLastError() == ERROR_INSUFFICIENT_BUFFER) // 缓冲区不足
		{
			lengthA = ::WideCharToMultiByte( codePage, 0, sz, lengthW, NULL, 0, NULL, NULL);
			strconv_allocMemory(&str, lengthA, (char*)0, 0);
			::WideCharToMultiByte(codePage, 0, sz, lengthW, str, lengthA, NULL, NULL);
		}			
	}
#elif PLATFORM == PLATFORM_UNIX
	wcstombs(str, sz, lengthA);
#endif
	return str;
}

void conv_free(void* sz)
{
	if (sz)
	{
		free(sz);
	}
}