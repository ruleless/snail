#include "timestamp.h"

#ifdef USE_RDTSC
TimingMethod gTimingMethod = RDTSC_TIMING_METHOD;
#else // USE_RDTSC
const TimingMethod DEFAULT_TIMING_METHOD = GET_TIME_TIMING_METHOD;
TimingMethod gTimingMethod = NO_TIMING_METHOD;
#endif // USE_RDTSC

const char* getTimingMethodName()
{
	switch (gTimingMethod)
	{
		case NO_TIMING_METHOD:
			return "none";
		case RDTSC_TIMING_METHOD:
			return "rdtsc";
		case GET_TIME_OF_DAY_TIMING_METHOD:
			return "gettimeofday";
		case GET_TIME_TIMING_METHOD:
			return "gettime";
		default:
			return "Unknown";
	}
}

#ifdef unix
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

static uint64 calcStampsPerSecond_rdtsc()
{
	struct timeval	tvBefore,	tvSleep = {0, 500000},	tvAfter;
	uint64 stampBefore,	stampAfter;

	gettimeofday(&tvBefore, NULL);
	gettimeofday(&tvBefore, NULL);

	gettimeofday(&tvBefore, NULL);
	stampBefore = timestamp();

	select(0, NULL, NULL, NULL, &tvSleep);

	gettimeofday(&tvAfter, NULL);
	gettimeofday(&tvAfter, NULL);

	gettimeofday(&tvAfter, NULL);
	stampAfter = timestamp();

	uint64 microDelta =
		(tvAfter.tv_usec + 1000000 - tvBefore.tv_usec) % 1000000;

	uint64 stampDelta = stampAfter - stampBefore;

	return (stampDelta * 1000000ULL) / microDelta;
}

static uint64 calcStampsPerSecond_gettime()
{
	return 1000000000ULL;
}

static uint64 calcStampsPerSecond_gettimeofday()
{
	return 1000000ULL;
}

static uint64 calcStampsPerSecond()
{
	static bool firstTime = true;
	if (firstTime)
	{
		firstTime = false;
	}

#ifdef USE_RDTSC
	return calcStampsPerSecond_rdtsc();
#else // USE_RDTSC
	if (gTimingMethod == RDTSC_TIMING_METHOD)
		return calcStampsPerSecond_rdtsc();
	else if (gTimingMethod == GET_TIME_OF_DAY_TIMING_METHOD)
		return calcStampsPerSecond_gettimeofday();
	else if (gTimingMethod == GET_TIME_TIMING_METHOD)
		return calcStampsPerSecond_gettime();
	else
	{
		char * timingMethod = getenv("TIMING_METHOD");
		if (!timingMethod)
		{
			gTimingMethod = DEFAULT_TIMING_METHOD;
		}
		else if (strcmp(timingMethod, "rdtsc") == 0)
		{
			gTimingMethod = RDTSC_TIMING_METHOD;
		}
		else if (strcmp(timingMethod, "gettimeofday") == 0)
		{
			gTimingMethod = GET_TIME_OF_DAY_TIMING_METHOD;
		}
		else if (strcmp(timingMethod, "gettime") == 0)
		{
			gTimingMethod = GET_TIME_TIMING_METHOD;
		}
		else
		{
// 			WARNING_MSG(fmt::format("calcStampsPerSecond: "
// 						 "Unknown timing method '%s', using clock_gettime.\n",
// 						 timingMethod));

			gTimingMethod = DEFAULT_TIMING_METHOD;
		}

		return calcStampsPerSecond();
	}
#endif // USE_RDTSC
}

uint64 stampsPerSecond_rdtsc()
{
	static uint64 stampsPerSecondCache = calcStampsPerSecond_rdtsc();
	return stampsPerSecondCache;
}

double stampsPerSecondD_rdtsc()
{
	static double stampsPerSecondCacheD = double(stampsPerSecond_rdtsc());
	return stampsPerSecondCacheD;
}

uint64 stampsPerSecond_gettimeofday()
{
	static uint64 stampsPerSecondCache = calcStampsPerSecond_gettimeofday();
	return stampsPerSecondCache;
}

double stampsPerSecondD_gettimeofday()
{
	static double stampsPerSecondCacheD = double(stampsPerSecond_gettimeofday());
	return stampsPerSecondCacheD;
}

#elif defined(_WIN32)

#include <windows.h>

#ifdef USE_RDTSC
static uint64 calcStampsPerSecond()
{	
	LARGE_INTEGER	tvBefore,	tvAfter;
	DWORD			tvSleep = 500;
	uint64 stampBefore,	stampAfter;
	
	Sleep(100);
	
	QueryPerformanceCounter(&tvBefore);
	QueryPerformanceCounter(&tvBefore);

	QueryPerformanceCounter(&tvBefore);
	stampBefore = timestamp();

	Sleep(tvSleep);

	QueryPerformanceCounter(&tvAfter);
	QueryPerformanceCounter(&tvAfter);

	QueryPerformanceCounter(&tvAfter);
	stampAfter = timestamp();

	uint64 countDelta = tvAfter.QuadPart - tvBefore.QuadPart;
	uint64 stampDelta = stampAfter - stampBefore;

	LARGE_INTEGER	frequency;
	QueryPerformanceFrequency(&frequency);

	return (uint64)((stampDelta * uint64(frequency.QuadPart) ) / countDelta);
}

#else // USE_RDTSC

static uint64 calcStampsPerSecond()
{
	LARGE_INTEGER rate;
	QueryPerformanceFrequency(&rate);
	return rate.QuadPart;
}

#endif // USE_RDTSC
#endif // unix


// 每秒cpu所耗时间
uint64 stampsPerSecond()
{
	static uint64 _stampsPerSecondCache = calcStampsPerSecond();
	return _stampsPerSecondCache;
}

// 每秒cpu所耗时间 double版本
double stampsPerSecondD()
{
	static double stampsPerSecondCacheD = double(stampsPerSecond());
	return stampsPerSecondCacheD;
}


double TimeStamp::toSeconds(uint64 stamps)
{
	return double(stamps) / stampsPerSecondD();
}

TimeStamp TimeStamp::fromSeconds(double seconds)
{
	return uint64(seconds * stampsPerSecondD());
}

double TimeStamp::inSeconds() const
{
	return toSeconds(stamp_);
}

void TimeStamp::setInSeconds(double seconds)
{
	stamp_ = fromSeconds(seconds);
}

TimeStamp TimeStamp::ageInStamps() const
{
	return timestamp() - stamp_;
}

double TimeStamp::ageInSeconds() const
{
	return toSeconds(this->ageInStamps());
}