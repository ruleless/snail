#include "math.h"

namespace math
{

void srandom(int seed)
{
	::srandom(seed);
}

int random(int low, int high)
{
	double r = (double)::random() * (1.0 / ((double)RAND_MAX + 1.0));
	r *= (double)(high - low) + 1.0;
	return (int)r + low;
}

}
