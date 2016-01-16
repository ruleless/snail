#include "concurrency.h"

static void default_op() {}

void (*pMainThreadIdleStartCallback)() = &default_op;
void (*pMainThreadIdleEndCallback)() = &default_op;
