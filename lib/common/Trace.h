#ifndef __TRACE_H__
#define __TRACE_H__

#include "Common.h"

#ifdef SUPPORT_TRACE

#include "Buffer.h"
#include "StrBuffer.h"

#include <iostream>
#include <string>
#include <list>

#if PLATFORM == PLATFORM_WIN32
#	include <windows.h>
#	include <process.h>
#endif


//--------------------------------------------------------------------------
enum TraceLevel // 消息级别
{
	levelInfo		= 0x01,
	levelTrace		= 0x02,
	levelWarning	= 0x04,
	levelError		= 0x08,
	levelEmphasis	= 0x10,

	levelAll = levelInfo|levelTrace|levelWarning|levelError|levelEmphasis,
};

class STrace
{
public:
	class Listener
	{
	protected:
		int mLevel;
		bool mHasTime;
	public:
		Listener()
			:mLevel(levelAll)
			,mHasTime(true)
		{
		}

		virtual ~Listener()
		{
		}

		bool hasTime() const
		{
			return mHasTime;
		}

		void hasTime(bool b)
		{
			mHasTime = b;
		}

		void setTraceLevel(int lvl)
		{
			mLevel = lvl; 
		}

		int getTraceLevel() const
		{
			return mLevel;
		}

		virtual void onTrace(const char* msg, const char* time, TraceLevel level)
		{
		}
	};
protected:
	struct _MSG
	{
		std::string msg;
		time_t time;
		TraceLevel level;
	};

	typedef std::list<_MSG> MsgList;
	typedef std::list<Listener *> ListenerList;

	ListenerList mSinks;
	int	mLevel;
	bool mHasTime;
	bool limitFrequency;

	volatile bool mbExit;
	bool mInited;
	THREAD_ID mTid;
	THREAD_MUTEX mMutex;
	THREAD_SINGNAL mCond;
	MsgList	mMsgs1;
	MsgList	mMsgs2;
	MsgList *mInlist;
	MsgList *mOutlist;
public:
	STrace();
	~STrace();

	bool initialise(int level, bool hasTime);
	void finalise();

	void _flushOutlist();

	int getTraceLevel() const;
	int setTraceLevel(int level);

	bool hasTime(bool b);

	bool setTraceLimitFrequency(bool limitFrequency);
	bool hasLimitFrequency() const;

	void registerTrace(Listener* sink);
	void unregisterTrace(Listener* sink);

	void output(const char* msg, TraceLevel level);

#if PLATFORM == PLATFORM_WIN32
	static unsigned __stdcall _traceProc(void *arg);	
#else
	static void* _traceProc(void* arg);
#endif
};

extern STrace* gTrace;
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// Trace API
extern void createTrace(int level = levelAll, bool hasTime = true);
extern void closeTrace();
extern int setTraceLevel(int level);
extern int getTraceLevel();
extern void setTraceHasTime(bool b);
extern bool setTraceHasLimitFrequency(bool limitFrequency);
extern bool hasLimitFrequency();
extern void registerTrace(STrace::Listener* sink);
extern void unregisterTrace(STrace::Listener* sink);
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// Set Output Target
extern STrace::Listener* output2Console(int level = levelAll, bool hasTime = true);
extern STrace::Listener* output2Html(const char* filename, int level = levelAll, bool hasTime = true);

#if PLATFORM == PLATFORM_WIN32
extern void addTraceToRichEdit(void* hWndRichEdit, const char* msg, TraceLevel level);
extern void dispatch2RichEdit(STrace::Listener* tl);
extern STrace::Listener* output2RichEdit(void* hwnd, int level = levelAll, bool hasTime = true);
#endif
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// output trace info
extern void output(const char* msg, TraceLevel level = levelTrace);
//--------------------------------------------------------------------------


//--------------------------------------------------------------------------
// Macro Define
#ifdef LIMIT_FREQUENCY // 控制每个输出点的最快频率是1秒
#	define LIMIT_FREQ(e)	static ulong s_lastTraceTime = GetTickCount(); if (!hasLimitFrequency() || (getTickCount() - s_lastTraceTime) > 1000) { s_lastTraceTime = getTickCount(); e}
#else
#	define LIMIT_FREQ(e)	e
#endif

#define Info(x)				{ if(getTraceLevel() & levelInfo)		{ LIMIT_FREQ(ostrbuf osb; osb<<x;					output(osb.c_str(), levelInfo);)} }
#define Trace(x)			{ if(getTraceLevel() & levelTrace)		{ LIMIT_FREQ(ostrbuf osb; osb<<x;					output(osb.c_str(), levelTrace);) }}
#define Warning(x)			{ if(getTraceLevel() & levelWarning)	{ LIMIT_FREQ(ostrbuf osb; osb<<"WARNING: "<<x;		output(osb.c_str(), levelWarning);) }}
#define Error(x)			{ if(getTraceLevel() & levelError)		{ LIMIT_FREQ(ostrbuf osb; osb<<"ERROR: "<<x;		output(osb.c_str(), levelError);) }}
#define Emphasis(x)			{ if(getTraceLevel() & levelEmphasis)	{ LIMIT_FREQ(ostrbuf osb; osb<<x;					output(osb.c_str(), levelEmphasis);) }}

#define InfoLn(x)			{ if(getTraceLevel() & levelInfo)		{ LIMIT_FREQ(ostrbuf osb; osb<<x<<"\n";				output(osb.c_str(), levelInfo);)} }
#define TraceLn(x)			{ if(getTraceLevel() & levelTrace)		{ LIMIT_FREQ(ostrbuf osb; osb<<x<<"\n";				output(osb.c_str(), levelTrace);) }}
#define WarningLn(x)		{ if(getTraceLevel() & levelWarning)	{ LIMIT_FREQ(ostrbuf osb; osb<<"WARNING: "<<x<<"\n";output(osb.c_str(), levelWarning);) }}
#define ErrorLn(x)			{ if(getTraceLevel() & levelError)		{ LIMIT_FREQ(ostrbuf osb; osb<<"ERROR: "<<x<<"\n";	output(osb.c_str(), levelError);) }}
#define EmphasisLn(x)		{ if(getTraceLevel() & levelEmphasis)	{ LIMIT_FREQ(ostrbuf osb; osb<<x<<"\n";				output(osb.c_str(), levelEmphasis);) }}

#define InfoOnce(x)			{ static bool f=true; if(f) { f=!f; Info(x);}}
#define TraceOnce(x)		{ static bool f=true; if(f) { f=!f; STrace(x);}}
#define WarningOnce(x)		{ static bool f=true; if(f) { f=!f; Warning(x);}}
#define ErrorOnce(x)		{ static bool f=true; if(f) { f=!f; Error(x);}}
#define EmphasisOnce(x)		{ static bool f=true; if(f) { f=!f; Emphasis(x);}}

#define InfoOnceLn(x)		{ static bool f=true; if(f) { f=!f; InfoLn(x);}}
#define TraceOnceLn(x)		{ static bool f=true; if(f) { f=!f; TraceLn(x);}}
#define WarningOnceLn(x)	{ static bool f=true; if(f) { f=!f; WarningLn(x);}}
#define ErrorOnceLn(x)		{ static bool f=true; if(f) { f=!f; ErrorLn(x);}}
#define EmphasisOnceLn(x)	{ static bool f=true; if(f) { f=!f; EmphasisLn(x);}}

#define Verify(x)			{ if(!(x)) { LIMIT_FREQ(ostrbuf osb; osb<<"VERIFY: "<<#x<<endl;output(osb.c_str(), levelError);) }}
//--------------------------------------------------------------------------

#else // #ifdef SUPPORT_TRACE

//--------------------------------------------------------------------------
// If We don't support trace
#define Info(x)
#define Trace(x)
#define Warning(x)
#define Error(x)
#define Emphasis(x)

#define InfoLn(x)
#define TraceLn(x)
#define WarningLn(x)
#define ErrorLn(x)
#define EmphasisLn(x)

#define InfoOnce(x)
#define TraceOnce(x)
#define WarningOnce(x)
#define ErrorOnce(x)
#define EmphasisOnce(x)

#define InfoOnceLn(x)
#define TraceOnceLn(x)
#define WarningOnceLn(x)
#define ErrorOnceLn(x)
#define EmphasisOnceLn(x)

#define Verify(x)		(x)
//--------------------------------------------------------------------------

#endif // #ifndef SUPPORT_TRACE

#endif // __TRACE_H__