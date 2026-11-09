#include <bits/time.h>
#include <stdint.h>
#include <time.h>

#define NS_TO_S 1000000000ULL

uint64_t now_nano(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t) ts.tv_sec * NS_TO_S + (uint64_t) ts.tv_nsec;
}
