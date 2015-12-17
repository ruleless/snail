#ifndef __TIMESTAMP_H__
#define __TIMESTAMP_H__

#include "common/platform.h"

#ifdef unix
//#define USE_RDTSC
#else
//#define USE_RDTSC
#endif // unix

enum TimingMethod
{
	RDTSC_TIMING_METHOD, // 自CPU上电以来所经过的时钟周期数,纳秒级的计时精度
	GET_TIME_OF_DAY_TIMING_METHOD,
	NO_TIMING_METHOD,
};

extern TimingMethod gTimingMethod;
extern const char* getTimingMethodName();

#ifdef unix

inline uint64 timestamp_rdtsc()
{
	uint32 rethi, retlo;
	__asm__ __volatile__ (
		"rdtsc":
		"=d"    (rethi),
		"=a"    (retlo)
						  );
	return uint64(rethi) << 32 | retlo; 
}

// 使用 gettimeofday. 测试大概比慢RDTSC20倍-600倍。
// 此外，有一个问题：2.4内核下，连续两次调用gettimeofday的可能返回一个结果是倒着走。
#include <sys/time.h>

inline uint64 timestamp_gettimeofday()
{
	timeval tv;
	gettimeofday( &tv, NULL );
	return 1000000ULL * uint64( tv.tv_sec ) + uint64( tv.tv_usec );
}

#include <time.h>
#include <asm/unistd.h>

inline uint64 timestamp_gettime()
{
	timespec tv;
	return 1000000000ULL * tv.tv_sec + tv.tv_nsec;
}

inline uint64 timestamp()
{
#ifdef USE_RDTSC
	return timestamp_rdtsc();
#else // USE_RDTSC
	if (gTimingMethod == RDTSC_TIMING_METHOD)
		return timestamp_rdtsc();
	else if (gTimingMethod == GET_TIME_OF_DAY_TIMING_METHOD)
		return timestamp_gettimeofday();
	else
		return timestamp_gettime();
#endif // USE_RDTSC
}

#elif defined(_WIN32)

#ifdef USE_RDTSC
#pragma warning (push)
#pragma warning (disable: 4035)
inline uint64 timestamp()
{
	__asm rdtsc
}
#pragma warning (pop)
#else // USE_RDTSC

#include <windows.h>

inline uint64 timestamp()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}

#endif // USE_RDTSC

#else
#    error Unsupported platform!
#endif

extern uint64 stampsPerSecond();
extern double stampsPerSecondD();

inline double stampsToSeconds( uint64 stamps )
{
	return double( stamps )/stampsPerSecondD();
}

class TimeStamp
{
  public:
	TimeStamp(uint64 stamps = 0) : stamp_(stamps) {}

	operator uint64 &() { return stamp_; }
	operator uint64() const { return stamp_; }
	
	uint64 stamp() { return stamp_; }

	double inSeconds() const;
	void setInSeconds(double seconds);

	TimeStamp ageInStamps() const;
	double ageInSeconds() const;

	static double toSeconds( uint64 stamps );
	static TimeStamp fromSeconds( double seconds );

	uint64 stamp_;
};

#endif // __TIMESTAMP_H__
