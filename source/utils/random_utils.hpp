
#ifndef RANDOM_UTILS_HPP
#define RANDOM_UTILS_HPP

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <random>

void  gRandSeed(std::uint32_t &_seed);
float gRandFloatNormalized(void);
float gRandFloatMax(const float &_max);
float gRandFloatMinMax(const float &_min, const float &_max);
float gRandFloat(const float &_min, const float &_max);

#endif // RANDOM_UTILS_HPP

