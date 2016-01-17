#include "mathutil.h"
#include <math.h>
#include <stdlib.h>

namespace math
{
void srandom(int seed)
{
#if PLATFORM == PLATFORM_WIN32
	srand(seed);
#else
	::srandom(seed);
#endif
}

int random(int low, int high)
{
#if PLATFORM == PLATFORM_WIN32
	double r = (double)::rand() * (1.0 / ((double)RAND_MAX + 1.0));
#else
	double r = (double)::random() * (1.0 / ((double)RAND_MAX + 1.0));
#endif
	r *= (double)(high - low) + 1.0;
	return (int)r + low;
}
}
