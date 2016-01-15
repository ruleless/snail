#ifndef __STRCONV_H__
#define __STRCONV_H__

#include "Common.h"

enum
{
	FIXED_CONV_BUF_LEN = 256,
};


// char* to wchar*
class strconv_a2w
{
private:
	wchar_t *m_psz;
	wchar_t	m_szBuffer[FIXED_CONV_BUF_LEN];
public:
	strconv_a2w(const char* sz, unsigned int codePage = -1);
	strconv_a2w(char* sz, unsigned int codePage = -1);
	strconv_a2w(const std::string& str, unsigned int codePage = -1);
	strconv_a2w(std::string& str, unsigned int codePage = -1);
	~strconv_a2w();

	operator wchar_t*() const { return m_psz; }
	wchar_t* str() const { return m_psz; }
private:
	void conv(const char* sz, unsigned int codePage);

	strconv_a2w();
	strconv_a2w(const strconv_a2w&);
	strconv_a2w& operator=(const strconv_a2w&);
};


// wchar* to char*
class strconv_w2a
{
private:
	char *m_psz;
	char m_szBuffer[FIXED_CONV_BUF_LEN*4];
public:
	strconv_w2a(const wchar_t* sz, unsigned int codePage = -1);
	strconv_w2a(wchar_t* sz, unsigned int codePage = -1);
	strconv_w2a(const std::wstring& str, unsigned int codePage = -1);
	strconv_w2a(std::wstring& str, unsigned int codePage = -1);
	~strconv_w2a();

	operator char*() const { return m_psz; }
	char* str() const { return m_psz; }
private:
	void conv(const wchar_t* sz, unsigned int codePage);

	strconv_w2a();
	strconv_w2a(const strconv_w2a&);
	strconv_w2a& operator=(const strconv_w2a&);
};


extern wchar_t* conv_a2w(const char* sz, unsigned int codePage = -1);
extern char* conv_w2a(const wchar_t* sz, unsigned int codePage = -1);
extern void conv_free(void* sz);


#ifndef CP_UTF8
#	define CP_UTF8 65001 
#endif

#define a2w(s)					(strconv_a2w((s)).str())
#define w2a(ws)					(strconv_w2a((ws)).str())
#define a2utf8(s)				(strconv_w2a(strconv_a2w((s)).str(), CP_UTF8).str())
#define w2utf8(ws)				(strconv_w2a((ws), CP_UTF8).str())
#define utf82a(s)				(strconv_w2a(strconv_a2w((s), CP_UTF8).str()).str())
#define utf82w(s)				(strconv_a2w((s), CP_UTF8).str())

#define a2w_ex(s)				(conv_a2w((s)))
#define w2a_ex(ws)				(conv_w2a((ws)))
#define a2utf8_ex(s)			(conv_w2a(a2w(s), CP_UTF8))
#define w2utf8_ex(s)			(conv_w2a((s), CP_UTF8))
#define utf82a_ex(s)			(conv_w2a(utf82w((s))))
#define utf82w_ex(s)			(conv_a2w((s), CP_UTF8))

#ifdef _UNICODE

#	define a2t					a2w
#	define w2t
#	define t2a					w2a
#	define t2w
#	define utf82t				utf82w
#	define t2utf8				w2utf8

#	define a2t_ex				a2w_ex
#	define w2t_ex			
#	define t2a_ex				w2a_ex
#	define t2w_ex
#	define utf82t_ex			utf82w_ex
#	define t2utf8_ex			w2utf8_ex

#	ifdef _UTF8
#		define _utf82t			utf82w
#		define _t2utf8			w2utf8
#		define _utf82t_ex		utf82w_ex
#		define _t2utf8_ex		w2utf8_ex
#		define _conv_free		conv_free
#	else
#		define _utf82t			a2w
#		define _t2utf8			w2a
#		define _utf82t_ex		utf82a_ex
#		define _t2utf8_ex		w2a_ex
#		define _conv_free		conv_free
#	endif

#else

#	define a2t
#	define w2t					w2a
#	define t2a
#	define t2w					a2w
#	define utf82t				utf82a
#	define t2utf8				a2utf8

#	define a2t_ex
#	define w2t_ex				w2a_ex
#	define t2a_ex
#	define t2w_ex				a2w_ex
#	define utf82t_ex			utf82a_ex
#	define t2utf8_ex			a2utf8_ex

#	ifdef _UTF8
#		define _utf82t			utf82a	
#		define _t2utf8			a2utf8
#		define _utf82t_ex		utf82a_ex
#		define _t2utf8_ex		a2utf8_ex
#		define _conv_free		conv_free
#	else
#		define _utf82t		
#		define _t2utf8		
#		define _utf82t_ex
#		define _t2utf8_ex
#		define _conv_free
#	endif

#endif // #ifdef _UNICODE

#endif // __STRCONV_H__