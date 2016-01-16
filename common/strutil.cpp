#include "common.h"
#include "strutil.h"
#include <algorithm>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>
#include <cctype>
#include "utf8cpp/utf8.h"
#include "MemoryStream.h"

int bytes2string(unsigned char *src, int srcsize, unsigned char *dst, int dstsize)     
{     
	if (dst != NULL)  
	{  
		*dst = 0;  
	}  

	if (src == NULL || srcsize <= 0 || dst == NULL || dstsize <= srcsize * 2)  
	{  
		return 0;  
	}  

	const char szTable[] = "0123456789ABCDEF";

	for(int i=0; i<srcsize; ++i)     
	{     
		*dst++ = szTable[src[i] >> 4];     
		*dst++ = szTable[src[i] & 0x0f];   
	}     

	*dst = 0;      
	return  srcsize * 2;     
}

int string2bytes(unsigned char* src, unsigned char* dst, int dstsize)     
{  
	if(src == NULL)
		return 0;  

	int iLen = strlen((char *)src);  
	if (iLen <= 0 || iLen%2 != 0 || dst == NULL || dstsize < iLen/2)  
	{  
		return 0;  
	}  

	iLen /= 2;  
	strToUpper((char *)src); 
	for (int i=0; i<iLen; ++i)  
	{  
		int iVal = 0;  
		unsigned char *pSrcTemp = src + i*2;  
		sscanf((char *)pSrcTemp, "%02x", &iVal);  
		dst[i] = (unsigned char)iVal;  
	}  

	return iLen;  
}

std::string toLower(const std::string& str) {
	std::string t = str;
	std::transform(t.begin(), t.end(), t.begin(), tolower);
	return t;
}

std::string toUpper(const std::string& str) {
	std::string t = str;
	std::transform(t.begin(), t.end(), t.begin(), toupper);
	return t;
}

std::string &__ltrim(std::string &s) 
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

std::string &__rtrim(std::string &s) 
{
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}


std::string __trim(std::string s) 
{
	return __ltrim(__rtrim(s));
}

// 字符串替换
int __replace(std::string& str,  const std::string& pattern,  const std::string& newpat) 
{ 
	int count = 0; 
	const size_t nsize = newpat.size(); 
	const size_t psize = pattern.size(); 

	for(size_t pos = str.find(pattern, 0);  
		pos != std::string::npos; 
		pos = str.find(pattern,pos + nsize)) 
	{ 
		str.replace(pos, psize, newpat); 
		count++; 
	} 

	return count; 
}

int __replace(std::wstring& str,  const std::wstring& pattern,  const std::wstring& newpat) 
{ 
	int count = 0; 
	const size_t nsize = newpat.size(); 
	const size_t psize = pattern.size(); 

	for(size_t pos = str.find(pattern, 0);  
		pos != std::wstring::npos; 
		pos = str.find(pattern,pos + nsize)) 
	{ 
		str.replace(pos, psize, newpat); 
		count++; 
	} 

	return count; 
}


std::vector< std::string > __splits(const std::string& s, const std::string& delim, const bool keep_empty) 
{
	std::vector< std::string > result;

	if (delim.empty()) {
		result.push_back(s);
		return result;
	}

	std::string::const_iterator substart = s.begin(), subend;

	while (true) {
		subend = std::search(substart, s.end(), delim.begin(), delim.end());
		std::string temp(substart, subend);
		if (keep_empty || !temp.empty()) {
			result.push_back(temp);
		}
		if (subend == s.end()) {
			break;
		}
		substart = subend + delim.size();
	}

	return result;
}

char* wchar2char(const wchar_t* ts, size_t* outlen)
{
	int len = (wcslen(ts) + 1) * 4/* Linux下sizeof(wchar_t) == 4, Windows下是2字节，这里取大的 */;
	char* ccattr =(char *)malloc(len);
	memset(ccattr, 0, len);

	size_t slen = wcstombs(ccattr, ts, len);

	if(outlen)
		*outlen = slen;

	return ccattr;
};

void wchar2char(const wchar_t* ts, MemoryStream* pStream)
{
	int len = (wcslen(ts) + 1) * 4/* Linux下sizeof(wchar_t) == 4, Windows下是2字节，这里取大的 */;
	pStream->data_resize(pStream->wpos() + len);
	size_t slen = wcstombs((char*)&pStream->data()[pStream->wpos()], ts, len);
	pStream->wpos(pStream->wpos() + slen + 1);
};

wchar_t* char2wchar(const char* cs, size_t* outlen)
{
	int len = (strlen(cs) + 1) * 4/* Linux下sizeof(wchar_t) == 4, Windows下是2字节，这里取大的 */;
	wchar_t* ccattr =(wchar_t *)malloc(len);
	memset(ccattr, 0, len);

	size_t slen = mbstowcs(ccattr, cs, len);

	if(outlen)
		*outlen = slen;

	return ccattr;
};

size_t utf8length(std::string& utf8str)
{
	try
	{
		return utf8::distance(utf8str.c_str(), 
			utf8str.c_str() + utf8str.size());
	}
	catch (std::exception)
	{
		utf8str = "";
		return 0;
	}
}

void utf8truncate(std::string& utf8str, size_t len)
{
	try
	{
		size_t wlen = utf8::distance(utf8str.c_str(), 
			utf8str.c_str() + utf8str.size());
		if (wlen <= len)
			return;

		std::wstring wstr;
		wstr.resize(wlen);
		utf8::utf8to16(utf8str.c_str(), utf8str.c_str() + 
			utf8str.size(), &wstr[0]);
		wstr.resize(len);

		char* oend = utf8::utf16to8(wstr.c_str(), 
			wstr.c_str() + wstr.size(), &utf8str[0]);

		utf8str.resize(oend - (&utf8str[0]));
	}
	catch (std::exception)
	{
		utf8str = "";
	}
}

bool utf82wchar(char const* utf8str, size_t csize, 
				wchar_t* wstr, size_t& wsize)
{
	try
	{
		size_t len = utf8::distance(utf8str, utf8str + csize);

		if (len > wsize)
		{
			if (wsize > 0)
				wstr[0] = L'\0';
			wsize = 0;
			return false;
		}

		wsize = len;
		utf8::utf8to16(utf8str, utf8str + csize, wstr);
		wstr[len] = L'\0';
	}
	catch (std::exception)
	{
		if (wsize > 0)
			wstr[0] = L'\0';
		wsize = 0;
		return false;
	}

	return true;
}

bool utf82wchar(const std::string& utf8str, std::wstring& wstr)
{
	try
	{
		size_t len = utf8::distance(utf8str.c_str(), 
			utf8str.c_str() + utf8str.size());
		wstr.resize(len);

		if (len)
			utf8::utf8to16(utf8str.c_str(), 
			utf8str.c_str() + utf8str.size(), &wstr[0]);
	}
	catch (std::exception)
	{
		wstr = L"";
		return false;
	}

	return true;
}

bool wchar2utf8(const wchar_t* wstr, size_t size, std::string& utf8str)
{
	try
	{
		std::string utf8str2;
		utf8str2.resize(size * 4);                          // allocate for most long case

		char* oend = utf8::utf16to8(wstr, wstr + size, &utf8str2[0]);
		utf8str2.resize(oend - (&utf8str2[0]));             // remove unused tail
		utf8str = utf8str2;
	}
	catch (std::exception)
	{
		utf8str = "";
		return false;
	}

	return true;
}

bool wchar2utf8(const std::wstring& wstr, std::string& utf8str)
{
	try
	{
		std::string utf8str2;
		utf8str2.resize(wstr.size() * 4);                   // allocate for most long case

		char* oend = utf8::utf16to8(wstr.c_str(), 
			wstr.c_str() + wstr.size(), &utf8str2[0]);

		utf8str2.resize(oend - (&utf8str2[0]));             // remove unused tail
		utf8str = utf8str2;
	}
	catch (std::exception)
	{
		utf8str = "";
		return false;
	}

	return true;
}
