#include "Trace.h"

#ifdef SUPPORT_TRACE

#include <list>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <iostream>

#if PLATFORM == PLATFORM_WIN32
#	include <richedit.h>
#	include <tchar.h>
#endif


//--------------------------------------------------------------------------
// Trace API
void createTrace(int level, bool hasTime)
{
	if (gTrace)
	{
		delete gTrace;
	}

	gTrace = new STrace();
	if (!gTrace->initialise(level, hasTime))
	{
		delete gTrace;
		gTrace = 0;
	}
}

void closeTrace()
{
	if (gTrace)
	{
		delete gTrace;
		gTrace = 0;
	}
}

int setTraceLevel(int level)
{
	return gTrace->setTraceLevel(level);
}

int getTraceLevel()
{
	return gTrace->getTraceLevel();
}

void setTraceHasTime(bool b)
{
	gTrace->hasTime(b);
}

bool setTraceHasLimitFrequency(bool limitFrequency)
{
	return gTrace->setTraceLimitFrequency(limitFrequency);
}

bool hasLimitFrequency()
{
	return gTrace->hasLimitFrequency();
}

void registerTrace(STrace::Listener* sink)
{
	gTrace->registerTrace(sink);
}

void unregisterTrace(STrace::Listener* sink)
{
	gTrace->unregisterTrace(sink);
}
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// 输出到控制台
class TraceConsole : public STrace::Listener
{
public:
	virtual void onTrace(const char* msg, const char* time, TraceLevel level)
	{
		assert(msg != NULL);

		if (time && hasTime())
			std::cout<<time;
		std::cout<<msg;
	}
};

STrace::Listener* output2Console(int level, bool hasTime)
{
	TraceConsole* sink = new TraceConsole();
	sink->setTraceLevel(level);
	sink->hasTime(hasTime);
	registerTrace(sink);
	return sink;
}
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// 输出到Html文件
class TraceHtmlFile : public STrace::Listener
{
	FILE* mFile;
public:
	TraceHtmlFile() : mFile(0) {}

	~TraceHtmlFile()
	{
		if (mFile)
		{
			fputs("</font>\n</body>\n</html>", mFile);
			fclose(mFile);
		}
	}

	bool create(const char* filename, bool bWrite)
	{
		if(bWrite)
		{
			mFile = fopen(filename, "wt");
		}
		else
		{
			mFile = fopen(filename, "at");
		}

		if (!mFile)
		{
			return false;
		}
		assert(mFile != 0);

#ifdef _UNICODE
		uchar BOM[3] = {0xEF,0xBB,0xBF};
		fwrite(BOM, sizeof(BOM)/sizeof(uchar), 1, mFile);
		fputs(
			"<html>\n"
			"<head>\n"
			"<meta http-equiv=\"content-type\" content=\"text/html; charset=utf8\">\n"
			"<title>Html Output</title>\n"
			"</head>\n"
			"<body>\n<font face=\"Fixedsys\" size=\"2\" color=\"#0000FF\">\n", mFile);
#else
		fputs(
			"<html>\n"
			"<head>\n"
			"<meta http-equiv=\"content-type\" content=\"text/html; charset=gb2312\">\n"
			"<title>Html Output</title>\n"
			"</head>\n"
			"<body>\n<font face=\"Fixedsys\" size=\"2\" color=\"#0000FF\">\n", mFile);
#endif
		return true;
	}

	virtual void onTrace(const char* msg, const char* time, TraceLevel level)
	{
		assert(msg != NULL);

		static const mchar* color[] = 
		{
			0,
			"<font color=\"#000000\">", // Info
			"<font color=\"#0000FF\">",	// Trace
			0,
			"<font color=\"#FF00FF\">",	// Warning
			0,0,0,
			"<font color=\"#FF0000\">",	// Error
			0,0,0,0,0,0,0,
			"<font color=\"#FFFF00\" style =\"background-color:#6a3905;\">", // Emphasis
		};

		fputs(color[(int)level], mFile);

		if (time && hasTime())
		{
			fputs(time, mFile);
		}

		char* pStart = (char *)msg;
		char* pPos = pStart;
		char* pEnd = pStart + strlen(pStart) - sizeof(char);

		while (pPos <= pEnd)
		{
			if (*pPos == '\n') // 换行
			{
				if (pStart < pPos)
				{
					fwrite(pStart, pPos - pStart, 1, mFile);
				}
				fputs("<br>", mFile);

				pStart = ++pPos;
			}
			else
			{
				pPos ++;
			}
		}

		if (pStart < pPos)
			fwrite(pStart, pPos - pStart, 1, mFile);

		fputs("</font>\n", mFile);

		fflush(mFile);
	}
};

STrace::Listener* output2Html(const tchar* filename, int level, bool hasTime)
{
	TraceHtmlFile* sink = new TraceHtmlFile();
	if (!sink->create(filename, true))
	{
		delete sink;
		return 0;
	}
	sink->setTraceLevel(level);
	sink->hasTime(hasTime);
	registerTrace(sink);
	return sink;
}
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// windows RichEdit
#if PLATFORM == PLATFORM_WIN32
#ifdef _DEBUG
#	define MAX_RICHEDIT_MESSAGE_LEN	(256 * 1024) // RichEdit中最大容纳长度
#else
#	define MAX_RICHEDIT_MESSAGE_LEN	(128 * 1024)
#endif

class TraceRichEdit : public STrace::Listener
{
private:
	void* mHwnd;
public:
	TraceRichEdit(void* hwnd) : mHwnd(hwnd) {}

	virtual void onTrace(const char* msg, const char* time, TraceLevel level)
	{
		if (!mHwnd)
			return;

		if (time && hasTime())
			addTraceToRichEdit(mHwnd, time, level);
		addTraceToRichEdit(mHwnd, msg, level);
	}

	void dispatch()
	{
	}
};

void addTraceToRichEdit(void* hWndRichEdit, const char* msg, TraceLevel level)
{
	assert(msg != NULL);

	if (!msg || *msg == 0)
		return;

	HWND hWnd = (HWND)hWndRichEdit;
	if (hWnd == NULL || !::IsWindow(hWnd))
		return;

	// GetSel
	CHARRANGE crOld;
	::SendMessage(hWnd, EM_EXGETSEL, 0, (LPARAM)&crOld);

	// GetTextLength
	int nLen = (int)::SendMessage(hWnd, WM_GETTEXTLENGTH, NULL, NULL);
	int nStrLen = (int)_tcslen(msg);
	CHARRANGE cr;
	if (nLen + nStrLen > MAX_RICHEDIT_MESSAGE_LEN)
	{
		// SetSel
		cr.cpMin = 0;
		cr.cpMax = nLen + nStrLen - MAX_RICHEDIT_MESSAGE_LEN; //+ (MAX_RICHEDIT_MESSAGE_LEN>>5);
		::SendMessage(hWnd, EM_EXSETSEL, 0, (LPARAM)&cr);
		// ReplaceSel
		::SendMessage(hWnd, EM_REPLACESEL, (WPARAM)0, (LPARAM)"");
		// GetTextLength
		nLen = (int)::SendMessage(hWnd, WM_GETTEXTLENGTH, NULL, NULL);
	}

	// SetSel
	cr.cpMin = nLen;
	cr.cpMax = nLen;
	::SendMessage(hWnd, EM_EXSETSEL, 0, (LPARAM)&cr);

	// SetSelectionCharFormat
	CHARFORMAT2 cf;
	memset(&cf, 0, sizeof(CHARFORMAT2));
	cf.dwMask = CFM_COLOR | CFM_CHARSET | CFM_SIZE 
		| CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT |  CFM_UNDERLINE | CFM_LINK | CFM_SHADOW;
	cf.dwEffects = 0;
	cf.bCharSet = GB2312_CHARSET;
	static const COLORREF cls[] = 
	{
		0, 
		RGB(0,0,0),		// Info
		RGB(0,0,255),	// Trace
		0, 
		RGB(255,0,255), // Warning
		0,0,0,
		RGB(255,0,0),	// Error
		0,0,0,0,0,0,0,
		RGB(255,255,0)	// Emphasis
	};
	if (level == levelEmphasis)
	{
		cf.dwMask |= CFM_BACKCOLOR;
		cf.crBackColor = RGB(106,57,5);
	}
	cf.crTextColor = cls[(int)level];
	cf.yHeight = 9 * 20;
	cf.cbSize = sizeof(CHARFORMAT2);
	::SendMessage(hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

	// ReplaceSel
	::SendMessage(hWnd, EM_REPLACESEL, (WPARAM) 0, (LPARAM)msg);

	if (crOld.cpMax > crOld.cpMin)
	{
		// SetSel
		::SendMessage(hWnd, EM_EXSETSEL, 0, (LPARAM)&crOld);
	}

	// Scroll lines
	SCROLLINFO ScrollInfo;
	ScrollInfo.cbSize = sizeof(SCROLLINFO);
	ScrollInfo.fMask = SIF_ALL;
	::GetScrollInfo(hWnd, SB_VERT, &ScrollInfo);

	int nTotalLine = (int)::SendMessage(hWnd, EM_GETLINECOUNT, 0, 0);
	if (nTotalLine > 0)
	{
		int nEachLineHeihgt = ScrollInfo.nMax / nTotalLine;
		if (nEachLineHeihgt > 0)
		{
			int nUpLine = 0;
			if (nTotalLine > 0 && ScrollInfo.nMax > 0 && nEachLineHeihgt > 0)
				nUpLine = (ScrollInfo.nMax - ScrollInfo.nPos - (ScrollInfo.nPage - 1)) / nEachLineHeihgt;
			if (nUpLine > 0)
				::SendMessage(hWnd, EM_LINESCROLL, 0, nUpLine);
		}
	}
}

void dispatch2RichEdit(STrace::Listener* tl)
{
	TraceRichEdit* tre = (TraceRichEdit*)tl;
	if (tre)
	{
		tre->dispatch();
	}
}

STrace::Listener* output2RichEdit(void* hwnd, int level, bool hasTime)
{
	TraceRichEdit* sink = new TraceRichEdit(hwnd);
	sink->setTraceLevel(level);
	sink->hasTime(hasTime);
	registerTrace(sink);
	return sink;
}
#endif
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
void output(const tchar* msg, TraceLevel level)
{
	if (NULL == gTrace)
	{
#if PLATFORM == PLATFORM_WIN32
#ifdef _DEBUG
		if (::IsDebuggerPresent())
		{
			::OutputDebugString(msg);
		}
#endif
#endif
	}
	else
	{
		gTrace->output(msg, level);
	}
}
//--------------------------------------------------------------------------

#endif //  #ifdef SUPPORT_TRACE