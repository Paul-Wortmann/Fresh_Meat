
#ifndef SLEEP_HPP
#define SLEEP_HPP

#include <sched.h>
#include <unistd.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#endif // defined

void gSleep(double time);

#endif // SLEEP_HPP
