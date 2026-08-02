
#include "sleep_utils.hpp"

void gSleep(double time)
{
    #if defined(_WIN32) || defined(_WIN64)
        Sleep((DWORD)(time*1000));
    #endif // defined

    #if defined(__linux__)
    if (time == 0.0)
    {
        sched_yield(); // sched.h
    }
    else
    {
        usleep((useconds_t)(time*1000000)); // unistd.h
    }
    #endif
}
