#ifndef __TRACE_H__
#define __TRACE_H__

#include "Common.h"

#ifdef SUPPORT_TRACE

#include "Buffer.h"


// 消息级别
enum TraceLevel
{
	levelInfo		= 0x01,
	levelTrace		= 0x02,
	levelWarning	= 0x04,
	levelError		= 0x08,
	levelEmphasis	= 0x10,

	levelAll = levelInfo|levelTrace|levelWarning|levelError|levelEmphasis,
};


// 消息接收器
class TraceListener
{
protected:
	int level;
	bool has_time;
public:
	TraceListener() : level(levelAll), has_time(false) {}
	virtual ~TraceListener() {}

	void setTraceLevel(int lvl)		{ level = lvl; }
	int getTraceLevel() const		{ return level; }
	void setHasTime(bool hasTime)	{ has_time = hasTime; }
	bool hasTime() const			{ return has_time; }
	virtual void onTrace(const char* msg, const char* time, TraceLevel level) {}
};


extern void createTrace(int level = levelAll, bool hasTime = true, bool threadSafe = true);
extern int setTraceLevel(int level);
extern int getTraceLevel();
extern int trackTraceContextMenu(int level);
extern bool setTraceHasTime(bool hasTime);
extern bool setTraceHasFileLine(bool hasFileLine);
extern bool setTraceHasLimitFrequency(bool limitFrequency);
extern bool hasLimitFrequency();
extern size_t setTraceMessageThreshold(size_t threshold);
extern size_t getTraceMessageThreshold();
extern void registerTrace(TraceListener* sink);
extern void unregisterTrace(TraceListener* sink);
extern void closeTrace();

extern TraceListener* output2Console(int level = levelAll, bool hasTime = false);
extern TraceListener* output2File(const char* filename = NULL, int level = levelAll, bool hasTime = false);
extern TraceListener* output2RichEdit(void* hwnd, int level = levelAll, bool hasTime = false);
extern TraceListener* output2Socket(const char* host = "127.0.0.1", ushort port = 1234, bool hasTime = false);

extern void addTraceToRichEdit(void* hWndRichEdit, const char* msg, TraceLevel level);
extern void dispatch2RichEdit(TraceListener* tl);

extern void output(const char* msg, TraceLevel level = levelTrace, const char* file = 0, int line = 0);

extern TraceListener* crash_output2File(const char* filename);
extern void crash_output(const char* msg, TraceLevel level = levelTrace, const char* file = 0, int line = 0);

// 设置lua的路径
extern void setTraceLuaPath(const char* path);

#ifdef LIMIT_FREQUENCY // 控制每个输出点的最快频率是1秒
#define LIMIT_FREQ(e)	static rkt::ulong _last_trace_tick = GetTickCount(); if (!rkt::hasLimitFrequency() || (GetTickCount() - _last_trace_tick) > 1000) { _last_trace_tick = GetTickCount(); e}
#else
#define LIMIT_FREQ(e)	e
#endif


#define Trace(x)			{ if(rkt::getTraceLevel() & rkt::levelTrace)	{ LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<x;rkt::output(o_s_b.c_str(), rkt::levelTrace, __FILE__, __LINE__);)} }
#define Warning(x)			{ if(rkt::getTraceLevel() & rkt::levelWarning)	{ LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<"WARNING: "<<x;rkt::output(o_s_b.c_str(), rkt::levelWarning, __FILE__, __LINE__);)} }
#define Error(x)			{ if(rkt::getTraceLevel() & rkt::levelError)	{ LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<"ERROR: "<<x;rkt::output(o_s_b.c_str(), rkt::levelError, __FILE__, __LINE__);/*debugBreakEx(o_s_b.c_str());*/)} }
#define Emphasis(x)			{ if(rkt::getTraceLevel() & rkt::levelEmphasis)	{ LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<x;rkt::output(o_s_b.c_str(), rkt::levelEmphasis, __FILE__, __LINE__);)} }

#define TraceLn(x)			{ if(rkt::getTraceLevel() & rkt::levelTrace)	{ LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<x<<rkt::endl;rkt::output(o_s_b.c_str(), rkt::levelTrace, __FILE__, __LINE__);)} }
#define WarningLn(x)		{ if(rkt::getTraceLevel() & rkt::levelWarning)	{ LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<"WARNING: "<<x<<rkt::endl;rkt::output(o_s_b.c_str(), rkt::levelWarning, __FILE__, __LINE__);)} }
#define ErrorLn(x)			{ if(rkt::getTraceLevel() & rkt::levelError)	{ LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<"ERROR: "<<x<<rkt::endl;rkt::output(o_s_b.c_str(), rkt::levelError, __FILE__, __LINE__);/*debugBreakEx(o_s_b.c_str());*/)} }
#define EmphasisLn(x)		{ if(rkt::getTraceLevel() & rkt::levelEmphasis)	{ LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<x<<rkt::endl;rkt::output(o_s_b.c_str(), rkt::levelEmphasis, __FILE__, __LINE__);)} }

#define TraceOnce(x)		{static bool f=true;if(f){f=!f;Trace(x);}}
#define WarningOnce(x)		{static bool f=true;if(f){f=!f;Warning(x);}}
#define ErrorOnce(x)		{static bool f=true;if(f){f=!f;Error(x);}}
#define EmphasisOnce(x)		{static bool f=true;if(f){f=!f;Emphasis(x);}}

#define TraceOnceLn(x)		{static bool f=true;if(f){f=!f;TraceLn(x);}}
#define WarningOnceLn(x)	{static bool f=true;if(f){f=!f;WarningLn(x);}}
#define ErrorOnceLn(x)		{static bool f=true;if(f){f=!f;ErrorLn(x);}}
#define EmphasisOnceLn(x)	{static bool f=true;if(f){f=!f;EmphasisLn(x);}}

#define crash_ErrorLn(x)	{ if(rkt::getTraceLevel() & rkt::levelError){ rkt::ostrbuf o_s_b;o_s_b<<"ERROR: "<<x<<rkt::endl;rkt::crash_output(o_s_b.c_str(), rkt::levelError, __FILE__, __LINE__);} }

#ifdef SUPPORT_TRACE
#	define Info(x)			{ if(rkt::getTraceLevel() & rkt::levelInfo)	{ LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<x;rkt::output(o_s_b.c_str(), rkt::levelInfo, __FILE__, __LINE__);)} }
#	define InfoLn(x)		{ if(rkt::getTraceLevel() & rkt::levelInfo)	{ LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<x<<rkt::endl;rkt::output(o_s_b.c_str(), rkt::levelInfo, __FILE__, __LINE__);)} }
#	define InfoOnce(x)		{static bool f=true;if(f){f=!f;Info(x);}}
#	define InfoOnceLn(x)	{static bool f=true;if(f){f=!f;InfoLn(x);}}
#	define Assert(x)		{if(!(x)){LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<"ASSERT: "<<#x<<rkt::endl;rkt::output(o_s_b.c_str(), rkt::levelError, __FILE__, __LINE__);) debugBreakEx(#x);}}
#	define Verify(x)		{if(!(x)){LIMIT_FREQ(rkt::ostrbuf o_s_b;o_s_b<<"VERIFY: "<<#x<<rkt::endl;rkt::output(o_s_b.c_str(), rkt::levelError, __FILE__, __LINE__);)}}
#else
#	define Info(x)
#	define InfoLn(x)
#	define InfoOnce(x)
#	define InfoOnceLn(x)
#	define Assert(x)
#	define Verify(x)		(x)
#endif


#ifdef ASSERT
#undef ASSERT
#endif

#ifdef VERIFY
#undef VERIFY
#endif

#define ASSERT Assert
#define VERIFY Verify

#else // #ifdef SUPPORT_TRACE

#define Trace(x)
#define Warning(x)
#define Error(x)
#define Emphasis(x)

#define TraceLn(x)
#define WarningLn(x)
#define ErrorLn(x)
#define EmphasisLn(x)

#define TraceOnce(x)
#define WarningOnce(x)
#define ErrorOnce(x)
#define EmphasisOnce(x)

#define TraceOnceLn(x)
#define WarningOnceLn(x)
#define ErrorOnceLn(x)
#define EmphasisOnceLn(x)

#define Info(x)
#define InfoLn(x)
#define InfoOnce(x)
#define InfoOnceLn(x)
#define Assert(x)
#define Verify(x)		(x)

#endif // #ifndef SUPPORT_TRACE

#define InitDebugInfo  Info
#define InitDebugInfoLn  InfoLn

#endif // __TRACE_H__