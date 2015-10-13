#ifndef __TIMING_H__
#define __TIMING_H__

#include <time.h>
#include <sys/time.h>

inline uint64_t 
rdtsc (void)
{
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

inline double 
wall_sec (void)
{
    struct timeval t;
    gettimeofday (&t, NULL);
    return (double)t.tv_sec + ((double)t.tv_usec/1000000.0);
}

#endif
