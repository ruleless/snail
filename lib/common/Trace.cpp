#include "Trace.h"
#include "strconv.h"

#ifdef SUPPORT_TRACE

#include <list>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <iostream>

#if PLATFORM == PLATFORM_WIN32
#	include <richedit.h>
#endif


// 输出到控制台
class TraceConsole : public TraceListener
{
public:
	virtual void onTrace(const tchar* msg, const tchar* time, TraceLevel level)
	{
		assert(msg != NULL);

#ifdef _UNICODE
		if (time && hasTime())
			std::wcout<<time;
		std::wcout<<msg;
#else
		if (time && hasTime())
			std::cout<<time;
		std::cout<<msg;
#endif
	}
};


// 输出到Html文件
class TraceHtmlFile : public TraceListener
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

	bool create(const tchar* filename, bool bWrite)
	{
		if(bWrite)
		{
			mFile = fopen(t2a(filename), "wt");
		}
		else
		{
			mFile = fopen(t2a(filename), "at");
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

	virtual void onTrace(const tchar* msg, const tchar* time, TraceLevel level)
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
			fputs(_t2utf8(time), mFile);
		}

#ifdef _UNICODE
		mchar* new_msg = conv_w2a(msg, CP_UTF8);
		mchar* pStart = (mchar *)new_msg;
#else
		mchar* pStart = (mchar *)msg;
#endif
		mchar* pPos = pStart;
		mchar* pEnd = pStart + strlen(pStart) - sizeof(mchar);

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

#ifdef _UNICODE
		conv_free(new_msg);
#endif

		fputs("</font>\n", mFile);

		fflush(mFile);
	}
};


#ifdef _DEBUG
#	define MAX_RICHEDIT_MESSAGE_LEN	(256 * 1024)		/// RichEdit中最大容纳长度
#else // _DEBUG
#	define MAX_RICHEDIT_MESSAGE_LEN	(128 * 1024)
#endif

RKT_API void addTraceToRichEdit(void* hWndRichEdit, const tchar* msg, TraceLevel level)
{
#if PLATFORM == PLATFORM_WIN32
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
#endif
}

class TraceRichEdit : public TraceListener
{
	void* mHwnd;
public:
	TraceRichEdit(void* hwnd) : mHwnd(hwnd) {}
	virtual void onTrace(const tchar* msg, const tchar* time, TraceLevel level)
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



//////////////////////////////////////////////////////////////////////////
// Trace Api
class STrace
{
public:
	struct _MSG
	{
		tstring msg;
		time_t time;
		TraceLevel level;
	};

	typedef std::list<_MSG>				MsgList;
	typedef std::list<TraceListener*>	ListenerList;
	ListenerList	mSinks;		/// 接收器列表
	int				level;		/// 输出级别
	bool			hasTime;	/// 是否显示时间
	bool			hasFileLine;/// 是否显示文件行号
	bool			limitFrequency;/// 是否限制同一个位置的输出频率

	volatile bool	isExit;		/// 退出标志
	Mutex			mutex;		/// 互斥体
	HANDLE			hThread;	/// 线程句柄
	HANDLE			hEvent;		/// 事件句柄
	MsgList			msgs1;		/// 消息列表1
	MsgList			msgs2;		/// 消息列表2
	MsgList*		inlist;		/// 用于输入的列表
	MsgList*		outlist;	/// 用于输出的列表
	bool			clearOutList;	/// 是否需要移除处理完的节点

public:
	STrace()
	{

	}
	~STrace()
	{
		close();
	}

	int getTraceLevel() const { return this->level; }
	int setTraceLevel(int level)
	{
		this->mutex.Lock();
		int old = this->level;
		this->level = level;
		this->mutex.Unlock();
		return old;
	}

	bool setTraceHasTime(bool hasTime)
	{
		this->mutex.Lock();
		bool old = this->hasTime;
		this->hasTime = hasTime;
		this->mutex.Unlock();
		return old;
	}

	bool setTraceLimitFrequency(bool limitFrequency)
	{
		this->mutex.Lock();
		bool old = this->limitFrequency;
		this->limitFrequency = limitFrequency;
		this->mutex.Unlock();
		return old;
	}

	bool hasLimitFrequency() const
	{
		return this->limitFrequency;
	}

	void registerTrace(TraceListener* sink)
	{
		if (!sink) return;
		this->mutex.Lock();
		this->mSinks.remove(sink); // if exist, remove it
		this->mSinks.push_back(sink);
		this->mutex.Unlock();
	}

	void unregisterTrace(TraceListener* sink)
	{
		if (!sink) return;
		this->mutex.Lock();
		for (STrace::ListenerList::iterator it=this->mSinks.begin(); it!=this->mSinks.end(); ++it)
		{
			if (*it == sink)
			{
				this->mSinks.erase(it);
				break;
			}
		}
		this->mutex.Unlock();
	}

	bool create(int level, bool hasTime);
	void close();
	void output(const tchar* msg, TraceLevel level, const mchar* file, int line);
	void swaplist();
	void output_outlist();
	static DWORD WINAPI TraceProc(LPVOID arg);
};
STrace* gTrace = 0;

bool STrace::create(int level, bool hasTime)
{
	this->level = level;
	this->hasTime = hasTime;
	this->hasFileLine = true;
	this->limitFrequency = false;

	this->isExit = false;
	this->hThread = this->hEvent = NULL;
	this->inlist = &this->msgs1;
	this->outlist = &this->msgs2;
	this->clearOutList = false;

	DWORD dwThread = 0;
	HANDLE hThread = ::CreateThread(NULL, 0, TraceProc, this, 0, &dwThread);
	if (hThread != NULL)
	{
		// 创建事件
		HANDLE hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		if (hEvent != NULL)
		{
			this->hThread = hThread;
			this->hEvent = hEvent;
			return true;
		}
	}
	return false;
}

void STrace::close()
{
	if (this->hThread)
	{
		this->isExit = true;
		::SetEvent(this->hEvent);
		::WaitForSingleObject(this->hThread, 10000L/*INFINITE*/);
		::CloseHandle(this->hThread);
		this->hThread = NULL;

		// 可能还有输入的信息没有输出
		swaplist();
		output_outlist();

		this->mutex.Lock();
		this->mutex.Unlock();

		if (this->hEvent)
			::CloseHandle(this->hEvent);
	}


	for (STrace::ListenerList::iterator it=this->mSinks.begin(); it!=this->mSinks.end(); ++it)
		delete *it;

	this->mSinks.clear();
}

void STrace::swaplist()
{
	// 交换生产/消费列表
#ifdef RKT_DEBUG
	this->mutex.Lock(__FILE__, __LINE__);
#else
	this->mutex.Lock();
#endif
	STrace::MsgList* outlist = this->inlist;
	this->inlist = this->outlist;
	this->outlist = outlist;

	this->mutex.Unlock();
}

void STrace::output_outlist()
{
	STrace::MsgList* outlist = this->outlist;
	if (/*!traceObj->clearOutList && */!outlist->empty())
	{
		// 输出
		tchar time_str[32] = {0};
		const tchar* p_time_str = 0;
		for (STrace::MsgList::iterator i=outlist->begin(); i!=outlist->end(); ++i)
		{
			STrace::_MSG& msgnode = *i;
			//if (gTrace->hasTime)
			if (msgnode.time)
			{
				_tcsftime(time_str, 32, _T("[%m%d %H:%M:%S] "), localtime(&msgnode.time));
				p_time_str = time_str;
			}
			else
				p_time_str = 0;

			for (STrace::ListenerList::iterator it=this->mSinks.begin(); it!=this->mSinks.end(); ++it)
			{
				TraceListener* obj = *it;
				if (obj->getTraceLevel() & msgnode.level)
					obj->onTrace(msgnode.msg.c_str(), p_time_str, msgnode.level);
			}
		}

		outlist->clear(); // 改到生产者线程去释放
	}
}

DWORD WINAPI STrace::TraceProc(LPVOID arg)
{
	STrace* traceObj = (STrace*)arg;
	if (!traceObj)
		return 1;

	while (!traceObj->isExit)
	{
		DWORD dwWaitResult = ::WaitForSingleObject(traceObj->hEvent, 10000L/*INFINITE*/);
		DWORD dwErr = ::GetLastError();
		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0:
			breakable;
			break; 
		case WAIT_TIMEOUT:
			breakable;
			break;
		case WAIT_ABANDONED:
			breakable;
			break;
		}

		// 交换生产/消费列表
		traceObj->swaplist();
		traceObj->output_outlist();

		sleep(10);
	}

	return 0;
}

void STrace::output(const tchar* msg, TraceLevel level, const mchar* file, int line)
{
	if (!msg) return;

	// 全局Trace设置是否支持该级别的输出
	if ((this->level & (int)level) == 0)
		return;

	// 输出到VC的输出框(不通过线程，立即输出，内置特征)
#ifdef RKT_DEBUG
	if (isDebuggerPresent())
	{
		if (this->hasFileLine)
		{
			ostrbuf osb;
			osb<<a2t(file)<<_T("(")<<line<<_T("): ")<<msg;
			::OutputDebugString(osb.c_str());
		}
		else
		{
			::OutputDebugString(msg);
		}
	}
#endif

	// 线程加锁
#ifdef RKT_DEBUG
	this->mutex.Lock(file, line);
#else
	this->mutex.Lock();
#endif
	/*
	if (this->clearOutList)
	{
	this->outlist->clear();
	this->clearOutList = false;
	}*/

	// 压入一个空的消息体到输入列表
	this->inlist->push_back(STrace::_MSG());

	// 填充节点数据
	STrace::_MSG& msgnode = this->inlist->back();
	msgnode.level = level;
	msgnode.time = this->hasTime ? time(0) : 0;
	msgnode.msg = msg;

	// 解锁
	this->mutex.Unlock();

	// 设置事件
	if (!::SetEvent(this->hEvent))
	{
		DWORD dwErr = ::GetLastError();
	}
}

RKT_API void createTrace(int level, bool hasTime, bool threadSafe)
{
	if (gTrace)
		delete gTrace;

	gTrace = new STrace;
	if (!gTrace->create(level, hasTime))
	{
		delete gTrace;
		gTrace = 0;
	}
}

RKT_API void closeTrace()
{
	if (gTrace)
	{
		delete gTrace;
		gTrace = 0;
	}
}

RKT_API int setTraceLevel(int level)
{
	if (!gTrace) createTrace();
	return gTrace ? gTrace->setTraceLevel(level) : 0;
}

RKT_API int getTraceLevel()
{
	if (!gTrace) createTrace();
	return gTrace ? gTrace->getTraceLevel() : 0;
}

RKT_API int trackTraceContextMenu(int level)
{
	int newTraceLevel = level;
	HMENU hPopupMenu = ::CreatePopupMenu();
	if (!hPopupMenu)
		return newTraceLevel;

	ulong check = MF_UNCHECKED;

	if (!::AppendMenuA(hPopupMenu, MF_STRING|MF_ENABLED, 1, "Disable All"))
		goto err;

	if (!::AppendMenuA(hPopupMenu, MF_SEPARATOR, 0, ""))
		goto err;

	check = (level & levelInfo) ? MF_CHECKED : MF_UNCHECKED;
	if (!::AppendMenuA(hPopupMenu, MF_STRING|MF_ENABLED|check, 2, "Infomation"))
		goto err;

	check = (level & levelTrace) ? MF_CHECKED : MF_UNCHECKED;
	if (!::AppendMenuA(hPopupMenu, MF_STRING|MF_ENABLED|check, 3, "Trace"))
		goto err;

	check = (level & levelWarning) ? MF_CHECKED : MF_UNCHECKED;
	if (!::AppendMenuA(hPopupMenu, MF_STRING|MF_ENABLED|check, 4, "Warning"))
		goto err;

	check = (level & levelError) ? MF_CHECKED : MF_UNCHECKED;
	if (!::AppendMenuA(hPopupMenu, MF_STRING|MF_ENABLED|check, 5, "Error"))
		goto err;

	check = (level & levelEmphasis) ? MF_CHECKED : MF_UNCHECKED;
	if (!::AppendMenuA(hPopupMenu, MF_STRING|MF_ENABLED|check, 6, "Emphasis"))
		goto err;

	if (!::AppendMenuA(hPopupMenu, MF_SEPARATOR, 0, ""))
		goto err;

	if (!::AppendMenuA(hPopupMenu, MF_STRING|MF_ENABLED, 7, "Enable All"))
		goto err;

	if (gTrace)
	{
		check = gTrace->limitFrequency ? MF_CHECKED : MF_UNCHECKED;
		if (!::AppendMenuA(hPopupMenu, MF_STRING|MF_ENABLED|check, 8, "Limit Frequency"))
			goto err;
	}

	POINT pos;
	::GetCursorPos(&pos);
	BOOL id = ::TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD, 
		pos.x, pos.y, 0, ::GetForegroundWindow(), 0);

	switch (id)
	{
	case 0:
		break;
	case 1:
		newTraceLevel = 0;
		break;
	case 2:
		if (level & levelInfo)
			newTraceLevel &= ~levelInfo;
		else
			newTraceLevel |= levelInfo;
		break;
	case 3:
		if (level & levelTrace)
			newTraceLevel &= ~levelTrace;
		else
			newTraceLevel |= levelTrace;
		break;
	case 4:
		if (level & levelWarning)
			newTraceLevel &= ~levelWarning;
		else
			newTraceLevel |= levelWarning;
		break;
	case 5:
		if (level & levelError)
			newTraceLevel &= ~levelError;
		else
			newTraceLevel |= levelError;
		break;
	case 6:
		if (level & levelEmphasis)
			newTraceLevel &= ~levelEmphasis;
		else
			newTraceLevel |= levelEmphasis;
		break;
	case 7:
		newTraceLevel = levelAll;
		break;
	case 8:
		if (gTrace)
			gTrace->setTraceLimitFrequency(!gTrace->hasLimitFrequency());
		break;
	}

err:
	::DestroyMenu(hPopupMenu);
	return newTraceLevel;
}

RKT_API bool setTraceHasTime(bool hasTime)
{
	if (!gTrace) createTrace();
	gTrace->mutex.Lock();
	bool old = gTrace->hasTime;
	gTrace->hasTime = hasTime;
	gTrace->mutex.Unlock();
	return old;
}

RKT_API bool setTraceHasFileLine(bool hasFileLine)
{
	if (!gTrace) createTrace();
	gTrace->mutex.Lock();
	bool old = gTrace->hasFileLine;
	gTrace->hasFileLine = hasFileLine;
	gTrace->mutex.Unlock();
	return old;
}

RKT_API bool setTraceHasLimitFrequency(bool limitFrequency)
{
	if (!gTrace) createTrace();
	return gTrace->setTraceLimitFrequency(limitFrequency);
}

RKT_API bool hasLimitFrequency()
{
	if (!gTrace) createTrace();
	return gTrace->hasLimitFrequency();
}

RKT_API void registerTrace(TraceListener* sink)
{
	if (!sink) return;
	if (!gTrace) createTrace();
	if (gTrace)
		gTrace->registerTrace(sink);
}

RKT_API void unregisterTrace(TraceListener* sink)
{
	if (!sink || !gTrace) return;
	gTrace->unregisterTrace(sink);
}

RKT_API TraceListener* output2Console(int level, bool hasTime)
{
	if (!gTrace) createTrace();
	if (gTrace)
	{
		TraceConsole* sink = new TraceConsole();
		sink->setTraceLevel(level);
		sink->setHasTime(hasTime);
		registerTrace(sink);
		return sink;
	}
	return 0;
}

RKT_API TraceListener* output2File(const tchar* filename, int level, bool hasTime)
{
	tchar fn[256];
	if (!filename)
	{
		_stprintf_s(fn, _T("%s\\log.html"), getWorkDir());
		filename = fn;
	}
	if (filename)
	{
		if (!gTrace) createTrace();
		TraceHtmlFile* sink = new TraceHtmlFile();
		if (!sink->create(filename, true))
		{
			delete sink;
			return 0;
		}
		sink->setTraceLevel(level);
		sink->setHasTime(hasTime);
		registerTrace(sink);
		return sink;
	}
	return 0;
}

RKT_API TraceListener* output2RichEdit(void* hwnd, int level, bool hasTime)
{
#ifdef RKT_WIN32
	if (!gTrace) createTrace();
	TraceRichEdit* sink = new TraceRichEdit(hwnd);
	sink->setTraceLevel(level);
	sink->setHasTime(hasTime);
	registerTrace(sink);
	return sink;
#else
	return 0;
#endif
}

RKT_API TraceListener* output2Socket(const tchar* host, ushort port, bool hasTime)
{
	return 0;
	if (host && port != 0)
	{
		if (!gTrace) createTrace();
	}
}

RKT_API void output(const tchar* msg, TraceLevel level, const mchar* file, int line)
{
	if (!msg) return;

	// 未创建Trace输出，则调用系统默认的调试信息输出
	if (!gTrace)
	{
#ifdef RKT_DEBUG
		if (isDebuggerPresent())
		{
			ostrbuf osb;
			osb<<a2t(file)<<_T("(")<<line<<_T("): ")<<msg;
			::OutputDebugString(osb.c_str());
		}
#endif
	}
	else
	{
		gTrace->output(msg, level, file, line);
	}
}

RKT_API void dispatch2RichEdit(TraceListener* tl)
{
	TraceRichEdit* tre = (TraceRichEdit*)tl;
	if (tre)
	{
		tre->dispatch();
	}
}

// crash_开头的方法都是特殊的trace，只有很少模块用得到，只为了记录服务器非法用的，为了这些信息不被普通信息淹没	
STrace* crash_Trace = 0;
void crash_createTrace(int level, bool hasTime, bool threadSafe)
{
	if(crash_Trace)
		delete crash_Trace;

	crash_Trace = new STrace;
	if (!crash_Trace->create(level, hasTime))
	{
		delete crash_Trace;
		crash_Trace = 0;
	}
}

RKT_API TraceListener* crash_output2File(const tchar* filename)
{
	if (!filename) return NULL;		
	if (!crash_Trace) crash_createTrace(levelAll, true, true);
	TraceHtmlFile* sink = new TraceHtmlFile();
	if (!sink->create(filename, false))
	{
		delete sink;
		return 0;
	}
	sink->setTraceLevel(levelAll);
	sink->setHasTime(true);
	crash_Trace->registerTrace(sink);

	return sink;
}

RKT_API void crash_output(const tchar* msg, TraceLevel level, const mchar* file, int line)
{
	if (!msg) return;

	// 未创建Trace输出，则调用系统默认的调试信息输出
	if (!crash_Trace)
	{
#ifdef RKT_DEBUG
		if (isDebuggerPresent())
		{
			ostrbuf osb;
			osb<<a2t(file)<<_T("(")<<line<<_T("): ")<<msg;
			::OutputDebugString(osb.c_str());
		}
#endif
	}
	else
	{
		crash_Trace->output(msg, level, file, line);
	}
}

#endif //  #ifdef SUPPORT_TRACE