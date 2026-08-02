
#include "thread_utils.hpp"

// A function to halt the thread from processing for a set period of time
// Function called with _us to denote microseconds
void gThreadSleep(const std::uint32_t &_us)
{
    std::this_thread::sleep_for(std::chrono::microseconds(_us));
};
