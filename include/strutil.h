#ifndef __STRUTIL_H__
#define __STRUTIL_H__
#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#if defined (unix)
#define __isnan isnan
#define __isinf isinf
#define __snprintf snprintf
#define __vsnprintf vsnprintf
#define __vsnwprintf vsnwprintf
#define __snwprintf swprintf
#define __stricmp strcasecmp
#define __strnicmp strncasecmp
#define __fileno fileno
#ifndef __va_copy
#	define __va_copy va_copy
#endif // __va_copy
#else // unix
#define __isnan _isnan
#define __isinf(x) (!_finite(x) && !_isnan(x))
#define __snprintf _snprintf
#define __vsnprintf _vsnprintf
#define __vsnwprintf _vsnwprintf
#define __snwprintf _snwprintf
#define __stricmp _stricmp
#define __strnicmp _strnicmp
#define __fileno _fileno
#define __va_copy(dst, src) dst = src

#define __strtoq _strtoi64
#define __strtouq _strtoui64
#define __strtoll _strtoi64
#define __strtoull _strtoui64
#define __atoll _atoi64
#endif // unix

class MemoryStream;

std::string &__ltrim(std::string &s);
std::string &__rtrim(std::string &s);
std::string __trim(std::string s);

int __replace(std::string& str,  const std::string& pattern,  const std::string& newpat);
int __replace(std::wstring& str,  const std::wstring& pattern,  const std::wstring& newpat);

// 转换为大写
inline char* strToUpper(char* s)
{
	assert(s != NULL);

	while(*s)
	{
		*s = toupper((unsigned char)*s);
		s++;
	};

	return s; 
}

// 转换为小写
inline char* strToLower(char* s)
{
	assert(s != NULL);

	while(*s)
	{
		*s = tolower((unsigned char)*s);
		s++;
	};

	return s; 
}

template<typename T>
inline void __split(const std::basic_string<T>& s, T c, std::vector< std::basic_string<T> > &v)
{
	if(s.size() == 0)
		return;

	typename std::basic_string< T >::size_type i = 0;
	typename std::basic_string< T >::size_type j = s.find(c);

	while(j != std::basic_string<T>::npos)
	{
		std::basic_string<T> buf = s.substr(i, j - i);

		if(buf.size() > 0)
			v.push_back(buf);

		i = ++j; j = s.find(c, j);
	}

	if(j == std::basic_string<T>::npos)
	{
		std::basic_string<T> buf = s.substr(i, s.length() - i);
		if(buf.size() > 0)
			v.push_back(buf);
	}
}

std::vector< std::string > __splits(const std::string& s, const std::string& delim, const bool keep_empty = true);

int bytes2string(unsigned char *pSrc, int nSrcLen, unsigned char *pDst, int nDstMaxLen);
int string2bytes(unsigned char* szSrc, unsigned char* pDst, int nDstMaxLen);

// utf-8	
char* wchar2char(const wchar_t* ts, size_t* outlen = NULL);
void wchar2char(const wchar_t* ts, MemoryStream* pStream);
wchar_t* char2wchar(const char* cs, size_t* outlen = NULL);

bool utf82wchar(const std::string& utf8str, std::wstring& wstr);

bool utf82wchar(char const* utf8str, size_t csize, wchar_t* wstr, size_t& wsize);
inline bool utf82wchar(const std::string& utf8str, wchar_t* wstr, size_t& wsize)
{
	return utf82wchar(utf8str.c_str(), utf8str.size(), wstr, wsize);
}

bool wchar2utf8(const std::wstring& wstr, std::string& utf8str);
bool wchar2utf8(const wchar_t* wstr, size_t size, std::string& utf8str);

size_t utf8length(const std::string& utf8str);
void utf8truncate(const std::string& utf8str, size_t len);

#endif // __STRUTIL_H__
