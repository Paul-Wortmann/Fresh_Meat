
#include "random_utils.hpp"

void gRandSeed(std::uint32_t &_seed)
{
    if (_seed == 0)
        _seed = static_cast <std::uint32_t> (time(0));
    srand(_seed);
}

float gRandFloatNormalized(void)
{
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}

float gRandFloatMax(const float &_max)
{
    return static_cast <float> (rand()) / ((static_cast <float> (RAND_MAX) / _max));
}

float gRandFloatMinMax(const float &_min, const float &_max)
{
    return _min + static_cast <float> (rand()) / (( static_cast <float> (RAND_MAX) / (_max - _min)));
}

float gRandFloat(const float &_min, const float &_max)
{
    // 1. Seed the engine using a non-deterministic source
    std::random_device randomDevice;

    // 2. Instantiate a Mersenne Twister engine with the seed
    std::mt19937 generator(randomDevice());

    // 3. Define a uniform distribution for real numbers in the range [-1.0, 1.0]
    std::uniform_real_distribution<float> distribution(_min, _max);

    // Generate and return a random number
    return distribution(generator);
}
