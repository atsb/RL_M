#define _POSIX_C_SOURCE 199309L

#include "includes/larnfunc.h"
#include "includes/io.h"
#include "includes/nap.h"
#include <time.h>

void nap(int milliseconds)
{
    struct timespec tc;
    tc.tv_sec = milliseconds / 1000;
    tc.tv_nsec = (milliseconds % 1000) * 1000000;
    nanosleep(&tc, NULL);
}
